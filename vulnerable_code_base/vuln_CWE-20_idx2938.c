static ssize_t generic_perform_write(struct file *file,
				struct iov_iter *i, loff_t pos)
{
	struct address_space *mapping = file->f_mapping;
	const struct address_space_operations *a_ops = mapping->a_ops;
	long status = 0;
	ssize_t written = 0;
	unsigned int flags = 0;
	if (segment_eq(get_fs(), KERNEL_DS))
		flags |= AOP_FLAG_UNINTERRUPTIBLE;
	do {
		struct page *page;
		pgoff_t index;		 
		unsigned long offset;	 
		unsigned long bytes;	 
		size_t copied;		 
		void *fsdata;
		offset = (pos & (PAGE_CACHE_SIZE - 1));
		index = pos >> PAGE_CACHE_SHIFT;
		bytes = min_t(unsigned long, PAGE_CACHE_SIZE - offset,
						iov_iter_count(i));
again:
		if (unlikely(iov_iter_fault_in_readable(i, bytes))) {
			status = -EFAULT;
			break;
		}
		status = a_ops->write_begin(file, mapping, pos, bytes, flags,
						&page, &fsdata);
		if (unlikely(status))
			break;
		pagefault_disable();
		copied = iov_iter_copy_from_user_atomic(page, i, offset, bytes);
		pagefault_enable();
		flush_dcache_page(page);
		status = a_ops->write_end(file, mapping, pos, bytes, copied,
						page, fsdata);
		if (unlikely(status < 0))
			break;
		copied = status;
		cond_resched();
		if (unlikely(copied == 0)) {
			bytes = min_t(unsigned long, PAGE_CACHE_SIZE - offset,
						iov_iter_single_seg_count(i));
			goto again;
		}
		iov_iter_advance(i, copied);
		pos += copied;
		written += copied;
		balance_dirty_pages_ratelimited(mapping);
	} while (iov_iter_count(i));
	return written ? written : status;
}