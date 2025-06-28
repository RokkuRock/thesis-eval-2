static ssize_t ext4_ext_direct_IO(int rw, struct kiocb *iocb,
			      const struct iovec *iov, loff_t offset,
			      unsigned long nr_segs)
{
	struct file *file = iocb->ki_filp;
	struct inode *inode = file->f_mapping->host;
	ssize_t ret;
	size_t count = iov_length(iov, nr_segs);
	loff_t final_size = offset + count;
	if (rw == WRITE && final_size <= inode->i_size) {
		iocb->private = NULL;
		EXT4_I(inode)->cur_aio_dio = NULL;
		if (!is_sync_kiocb(iocb)) {
			iocb->private = ext4_init_io_end(inode);
			if (!iocb->private)
				return -ENOMEM;
			EXT4_I(inode)->cur_aio_dio = iocb->private;
		}
		ret = blockdev_direct_IO(rw, iocb, inode,
					 inode->i_sb->s_bdev, iov,
					 offset, nr_segs,
					 ext4_get_block_write,
					 ext4_end_io_dio);
		if (iocb->private)
			EXT4_I(inode)->cur_aio_dio = NULL;
		if (ret != -EIOCBQUEUED && ret <= 0 && iocb->private) {
			ext4_free_io_end(iocb->private);
			iocb->private = NULL;
		} else if (ret > 0 && ext4_test_inode_state(inode,
						EXT4_STATE_DIO_UNWRITTEN)) {
			int err;
			err = ext4_convert_unwritten_extents(inode,
							     offset, ret);
			if (err < 0)
				ret = err;
			ext4_clear_inode_state(inode, EXT4_STATE_DIO_UNWRITTEN);
		}
		return ret;
	}
	return ext4_ind_direct_IO(rw, iocb, iov, offset, nr_segs);
}