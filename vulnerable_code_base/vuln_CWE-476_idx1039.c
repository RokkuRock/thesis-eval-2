static int chmd_read_headers(struct mspack_system *sys, struct mspack_file *fh,
			     struct mschmd_header *chm, int entire)
{
  unsigned int section, name_len, x, errors, num_chunks;
  unsigned char buf[0x54], *chunk = NULL, *name, *p, *end;
  struct mschmd_file *fi, *link = NULL;
  off_t offset, length;
  int num_entries;
  chm->files         = NULL;
  chm->sysfiles      = NULL;
  chm->chunk_cache   = NULL;
  chm->sec0.base.chm = chm;
  chm->sec0.base.id  = 0;
  chm->sec1.base.chm = chm;
  chm->sec1.base.id  = 1;
  chm->sec1.content  = NULL;
  chm->sec1.control  = NULL;
  chm->sec1.spaninfo = NULL;
  chm->sec1.rtable   = NULL;
  if (sys->read(fh, &buf[0], chmhead_SIZEOF) != chmhead_SIZEOF) {
    return MSPACK_ERR_READ;
  }
  if (EndGetI32(&buf[chmhead_Signature]) != 0x46535449) {
    return MSPACK_ERR_SIGNATURE;
  }
  if (mspack_memcmp(&buf[chmhead_GUID1], &guids[0], 32L) != 0) {
    D(("incorrect GUIDs"))
    return MSPACK_ERR_SIGNATURE;
  }
  chm->version   = EndGetI32(&buf[chmhead_Version]);
  chm->timestamp = EndGetM32(&buf[chmhead_Timestamp]);
  chm->language  = EndGetI32(&buf[chmhead_LanguageID]);
  if (chm->version > 3) {
    sys->message(fh, "WARNING; CHM version > 3");
  }
  if (sys->read(fh, &buf[0], chmhst3_SIZEOF) != chmhst3_SIZEOF) {
    return MSPACK_ERR_READ;
  }
  if (read_off64(&offset,           &buf[chmhst_OffsetHS0],  sys, fh) ||
      read_off64(&chm->dir_offset,  &buf[chmhst_OffsetHS1],  sys, fh) ||
      read_off64(&chm->sec0.offset, &buf[chmhst3_OffsetCS0], sys, fh))
  {
    return MSPACK_ERR_DATAFORMAT;
  }
  if (sys->seek(fh, offset, MSPACK_SYS_SEEK_START)) {
    return MSPACK_ERR_SEEK;
  }
  if (sys->read(fh, &buf[0], chmhs0_SIZEOF) != chmhs0_SIZEOF) {
    return MSPACK_ERR_READ;
  }
  if (read_off64(&chm->length, &buf[chmhs0_FileLen], sys, fh)) {
    return MSPACK_ERR_DATAFORMAT;
  }
  if (sys->seek(fh, chm->dir_offset, MSPACK_SYS_SEEK_START)) {
    return MSPACK_ERR_SEEK;
  }
  if (sys->read(fh, &buf[0], chmhs1_SIZEOF) != chmhs1_SIZEOF) {
    return MSPACK_ERR_READ;
  }
  chm->dir_offset = sys->tell(fh);
  chm->chunk_size = EndGetI32(&buf[chmhs1_ChunkSize]);
  chm->density    = EndGetI32(&buf[chmhs1_Density]);
  chm->depth      = EndGetI32(&buf[chmhs1_Depth]);
  chm->index_root = EndGetI32(&buf[chmhs1_IndexRoot]);
  chm->num_chunks = EndGetI32(&buf[chmhs1_NumChunks]);
  chm->first_pmgl = EndGetI32(&buf[chmhs1_FirstPMGL]);
  chm->last_pmgl  = EndGetI32(&buf[chmhs1_LastPMGL]);
  if (chm->version < 3) {
    chm->sec0.offset = chm->dir_offset + (chm->chunk_size * chm->num_chunks);
  }
  if (chm->sec0.offset > chm->length) {
    D(("content section begins after file has ended"))
    return MSPACK_ERR_DATAFORMAT;
  }
  if (chm->chunk_size < (pmgl_Entries + 2)) {
    D(("chunk size not large enough"))
    return MSPACK_ERR_DATAFORMAT;
  }
  if (chm->num_chunks == 0) {
    D(("no chunks"))
    return MSPACK_ERR_DATAFORMAT;
  }
  if (chm->num_chunks > 100000) {
    D(("more than 100,000 chunks"))
    return MSPACK_ERR_DATAFORMAT;
  }   
  if ((off_t)chm->chunk_size * (off_t)chm->num_chunks > chm->length) {
    D(("chunks larger than entire file"))
    return MSPACK_ERR_DATAFORMAT;
  }
  if ((chm->chunk_size & (chm->chunk_size - 1)) != 0) {
    sys->message(fh, "WARNING; chunk size is not a power of two");
  }
  if (chm->first_pmgl != 0) {
    sys->message(fh, "WARNING; first PMGL chunk is not zero");
  }
  if (chm->first_pmgl > chm->last_pmgl) {
    D(("first pmgl chunk is after last pmgl chunk"))
    return MSPACK_ERR_DATAFORMAT;
  }
  if (chm->index_root != 0xFFFFFFFF && chm->index_root >= chm->num_chunks) {
    D(("index_root outside valid range"))
    return MSPACK_ERR_DATAFORMAT;
  }
  if (!entire) {
    return MSPACK_ERR_OK;
  }
  if ((x = chm->first_pmgl) != 0) {
    if (sys->seek(fh,(off_t) (x * chm->chunk_size), MSPACK_SYS_SEEK_CUR)) {
      return MSPACK_ERR_SEEK;
    }
  }
  num_chunks = chm->last_pmgl - x + 1;
  if (!(chunk = (unsigned char *) sys->alloc(sys, (size_t)chm->chunk_size))) {
    return MSPACK_ERR_NOMEMORY;
  }
  errors = 0;
  while (num_chunks--) {
    if (sys->read(fh, chunk, (int)chm->chunk_size) != (int)chm->chunk_size) {
      sys->free(chunk);
      return MSPACK_ERR_READ;
    }
    if (EndGetI32(&chunk[pmgl_Signature]) != 0x4C474D50) continue;
    if (EndGetI32(&chunk[pmgl_QuickRefSize]) < 2) {
      sys->message(fh, "WARNING; PMGL quickref area is too small");
    }
    if (EndGetI32(&chunk[pmgl_QuickRefSize]) > 
	((int)chm->chunk_size - pmgl_Entries))
    {
      sys->message(fh, "WARNING; PMGL quickref area is too large");
    }
    p = &chunk[pmgl_Entries];
    end = &chunk[chm->chunk_size - 2];
    num_entries = EndGetI16(end);
    while (num_entries--) {
      READ_ENCINT(name_len);
      if (name_len > (unsigned int) (end - p)) goto chunk_end;
      if (name_len == 0) goto chunk_end;
      name = p; p += name_len;
      READ_ENCINT(section);
      READ_ENCINT(offset);
      READ_ENCINT(length);
      if ((offset == 0) && (length == 0)) {
	if ((name_len > 0) && (name[name_len-1] == '/')) continue;
      }
      if (section > 1) {
	sys->message(fh, "invalid section number '%u'.", section);
	continue;
      }
      if (!(fi = (struct mschmd_file *) sys->alloc(sys, sizeof(struct mschmd_file) + name_len + 1))) {
	sys->free(chunk);
	return MSPACK_ERR_NOMEMORY;
      }
      fi->next     = NULL;
      fi->filename = (char *) &fi[1];
      fi->section  = ((section == 0) ? (struct mschmd_section *) (&chm->sec0)
		                     : (struct mschmd_section *) (&chm->sec1));
      fi->offset   = offset;
      fi->length   = length;
      sys->copy(name, fi->filename, (size_t) name_len);
      fi->filename[name_len] = '\0';
      if (name[0] == ':' && name[1] == ':') {
	if (mspack_memcmp(&name[2], &content_name[2], 31L) == 0) {
	  if (mspack_memcmp(&name[33], &content_name[33], 8L) == 0) {
	    chm->sec1.content = fi;
	  }
	  else if (mspack_memcmp(&name[33], &control_name[33], 11L) == 0) {
	    chm->sec1.control = fi;
	  }
	  else if (mspack_memcmp(&name[33], &spaninfo_name[33], 8L) == 0) {
	    chm->sec1.spaninfo = fi;
	  }
	  else if (mspack_memcmp(&name[33], &rtable_name[33], 72L) == 0) {
	    chm->sec1.rtable = fi;
	  }
	}
	fi->next = chm->sysfiles;
	chm->sysfiles = fi;
      }
      else {
	if (link) link->next = fi; else chm->files = fi;
	link = fi;
      }
    }
  chunk_end:
    if (num_entries >= 0) {
      D(("chunk ended before all entries could be read"))
      errors++;
    }
  }
  sys->free(chunk);
  return (errors > 0) ? MSPACK_ERR_DATAFORMAT : MSPACK_ERR_OK;
}