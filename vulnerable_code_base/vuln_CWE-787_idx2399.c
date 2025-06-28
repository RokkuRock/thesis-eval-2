int sr_do_ioctl(Scsi_CD *cd, struct packet_command *cgc)
{
	struct scsi_device *SDev;
	struct scsi_sense_hdr sshdr;
	int result, err = 0, retries = 0;
	SDev = cd->device;
      retry:
	if (!scsi_block_when_processing_errors(SDev)) {
		err = -ENODEV;
		goto out;
	}
	result = scsi_execute(SDev, cgc->cmd, cgc->data_direction,
			      cgc->buffer, cgc->buflen,
			      (unsigned char *)cgc->sense, &sshdr,
			      cgc->timeout, IOCTL_RETRIES, 0, 0, NULL);
	if (driver_byte(result) != 0) {
		switch (sshdr.sense_key) {
		case UNIT_ATTENTION:
			SDev->changed = 1;
			if (!cgc->quiet)
				sr_printk(KERN_INFO, cd,
					  "disc change detected.\n");
			if (retries++ < 10)
				goto retry;
			err = -ENOMEDIUM;
			break;
		case NOT_READY:	 
			if (sshdr.asc == 0x04 &&
			    sshdr.ascq == 0x01) {
				if (!cgc->quiet)
					sr_printk(KERN_INFO, cd,
						  "CDROM not ready yet.\n");
				if (retries++ < 10) {
					ssleep(2);
					goto retry;
				} else {
					err = -ENOMEDIUM;
					break;
				}
			}
			if (!cgc->quiet)
				sr_printk(KERN_INFO, cd,
					  "CDROM not ready.  Make sure there "
					  "is a disc in the drive.\n");
			err = -ENOMEDIUM;
			break;
		case ILLEGAL_REQUEST:
			err = -EIO;
			if (sshdr.asc == 0x20 &&
			    sshdr.ascq == 0x00)
				err = -EDRIVE_CANT_DO_THIS;
			break;
		default:
			err = -EIO;
		}
	}
      out:
	cgc->stat = err;
	return err;
}