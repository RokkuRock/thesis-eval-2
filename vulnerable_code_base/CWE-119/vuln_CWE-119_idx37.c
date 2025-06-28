grub_disk_read (grub_disk_t disk, grub_disk_addr_t sector,
		grub_off_t offset, grub_size_t size, void *buf)
{
  char *tmp_buf;
  unsigned real_offset;
  if (grub_disk_adjust_range (disk, &sector, &offset, size) != GRUB_ERR_NONE)
    {
      grub_error_push ();
      grub_dprintf ("disk", "Read out of range: sector 0x%llx (%s).\n",
		    (unsigned long long) sector, grub_errmsg);
      grub_error_pop ();
      return grub_errno;
    }
  real_offset = offset;
  tmp_buf = grub_malloc (GRUB_DISK_SECTOR_SIZE << GRUB_DISK_CACHE_BITS);
  if (! tmp_buf)
    return grub_errno;
  while (size)
    {
      char *data;
      grub_disk_addr_t start_sector;
      grub_size_t len;
      grub_size_t pos;
      start_sector = sector & ~(GRUB_DISK_CACHE_SIZE - 1);
      pos = (sector - start_sector) << GRUB_DISK_SECTOR_BITS;
      len = ((GRUB_DISK_SECTOR_SIZE << GRUB_DISK_CACHE_BITS)
	     - pos - real_offset);
      if (len > size)
	len = size;
      data = grub_disk_cache_fetch (disk->dev->id, disk->id, start_sector);
      if (data)
	{
	  if (buf)
	    grub_memcpy (buf, data + pos + real_offset, len);
	  grub_disk_cache_unlock (disk->dev->id, disk->id, start_sector);
	}
      else
	{
	  if (start_sector + GRUB_DISK_CACHE_SIZE > disk->total_sectors
	      || (disk->dev->read) (disk, start_sector,
				    GRUB_DISK_CACHE_SIZE, tmp_buf)
	      != GRUB_ERR_NONE)
	    {
	      unsigned num;
	      char *p;
	      grub_errno = GRUB_ERR_NONE;
	      num = ((size + real_offset + GRUB_DISK_SECTOR_SIZE - 1)
		     >> GRUB_DISK_SECTOR_BITS);
	      p = grub_realloc (tmp_buf, num << GRUB_DISK_SECTOR_BITS);
	      if (!p)
		goto finish;
	      tmp_buf = p;
	      if ((disk->dev->read) (disk, sector, num, tmp_buf))
		{
		  grub_error_push ();
		  grub_dprintf ("disk", "%s read failed\n", disk->name);
		  grub_error_pop ();
		  goto finish;
		}
	      if (buf)
		grub_memcpy (buf, tmp_buf + real_offset, size);
	      if (disk->read_hook)
		while (size)
		  {
		    grub_size_t to_read;
		    to_read = size;
		    if (real_offset + to_read > GRUB_DISK_SECTOR_SIZE)
		      to_read = GRUB_DISK_SECTOR_SIZE - real_offset;
		    (disk->read_hook) (sector, real_offset,
				       to_read, disk->closure);
		    if (grub_errno != GRUB_ERR_NONE)
		      goto finish;
		    sector++;
		    size -= to_read;
		    real_offset = 0;
		  }
	      goto finish;
	    }
	  if (buf)
	    grub_memcpy (buf, tmp_buf + pos + real_offset, len);
	  grub_disk_cache_store (disk->dev->id, disk->id,
				 start_sector, tmp_buf);
	}
      if (disk->read_hook)
	{
	  grub_disk_addr_t s = sector;
	  grub_size_t l = len;
	  while (l)
	    {
	      (disk->read_hook) (s, real_offset,
				 ((l > GRUB_DISK_SECTOR_SIZE)
				  ? GRUB_DISK_SECTOR_SIZE
				  : l), disk->closure);
	      if (l < GRUB_DISK_SECTOR_SIZE - real_offset)
		break;
	      s++;
	      l -= GRUB_DISK_SECTOR_SIZE - real_offset;
	      real_offset = 0;
	    }
	}
      sector = start_sector + GRUB_DISK_CACHE_SIZE;
      if (buf)
	buf = (char *) buf + len;
      size -= len;
      real_offset = 0;
    }
 finish:
  grub_free (tmp_buf);
  return grub_errno;
}