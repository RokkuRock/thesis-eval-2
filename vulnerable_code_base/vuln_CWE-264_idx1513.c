static int sd_compat_ioctl(struct block_device *bdev, fmode_t mode,
			   unsigned int cmd, unsigned long arg)
{
	struct scsi_device *sdev = scsi_disk(bdev->bd_disk)->device;
	if (!scsi_block_when_processing_errors(sdev))
		return -ENODEV;
	if (sdev->host->hostt->compat_ioctl) {
		int ret;
		ret = sdev->host->hostt->compat_ioctl(sdev, cmd, (void __user *)arg);
		return ret;
	}
	return -ENOIOCTLCMD; 
}