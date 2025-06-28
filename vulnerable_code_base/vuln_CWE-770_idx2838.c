stream_read(pdfio_stream_t *st,		 
            char           *buffer,	 
            size_t         bytes)	 
{
  ssize_t	rbytes;			 
  if (st->filter == PDFIO_FILTER_NONE)
  {
    if (bytes > st->remaining)
      rbytes = _pdfioFileRead(st->pdf, buffer, st->remaining);
    else
      rbytes = _pdfioFileRead(st->pdf, buffer, bytes);
    if (rbytes > 0)
    {
      st->remaining -= (size_t)rbytes;
      if (st->crypto_cb)
        (st->crypto_cb)(&st->crypto_ctx, (uint8_t *)buffer, (uint8_t *)buffer, (size_t)rbytes);
    }
    return (rbytes);
  }
  else if (st->filter == PDFIO_FILTER_FLATE)
  {
    int	status;				 
    if (st->predictor == _PDFIO_PREDICTOR_NONE)
    {
      PDFIO_DEBUG("stream_read: No predictor.\n");
      if (st->flate.avail_in == 0)
      {
	if (sizeof(st->cbuffer) > st->remaining)
	  rbytes = _pdfioFileRead(st->pdf, st->cbuffer, st->remaining);
	else
	  rbytes = _pdfioFileRead(st->pdf, st->cbuffer, sizeof(st->cbuffer));
	if (rbytes <= 0)
	  return (-1);			 
	if (st->crypto_cb)
	  rbytes = (ssize_t)(st->crypto_cb)(&st->crypto_ctx, st->cbuffer, st->cbuffer, (size_t)rbytes);
	st->remaining      -= (size_t)rbytes;
	st->flate.next_in  = (Bytef *)st->cbuffer;
	st->flate.avail_in = (uInt)rbytes;
      }
      st->flate.next_out  = (Bytef *)buffer;
      st->flate.avail_out = (uInt)bytes;
      if ((status = inflate(&(st->flate), Z_NO_FLUSH)) < Z_OK)
      {
	_pdfioFileError(st->pdf, "Unable to decompress stream data: %s", zstrerror(status));
	return (-1);
      }
      return (st->flate.next_out - (Bytef *)buffer);
    }
    else if (st->predictor == _PDFIO_PREDICTOR_TIFF2)
    {
      size_t		pbpixel = st->pbpixel,
      			remaining = st->pbsize;
      unsigned char	*bufptr = (unsigned char *)buffer,
			*bufsecond = (unsigned char *)buffer + pbpixel,
			*sptr = st->psbuffer;
      PDFIO_DEBUG("stream_read: TIFF predictor 2.\n");
      if (bytes < st->pbsize)
      {
        _pdfioFileError(st->pdf, "Read buffer too small for stream.");
        return (-1);
      }
      st->flate.next_out  = (Bytef *)sptr;
      st->flate.avail_out = (uInt)st->pbsize;
      while (st->flate.avail_out > 0)
      {
	if (st->flate.avail_in == 0)
	{
	  if (sizeof(st->cbuffer) > st->remaining)
	    rbytes = _pdfioFileRead(st->pdf, st->cbuffer, st->remaining);
	  else
	    rbytes = _pdfioFileRead(st->pdf, st->cbuffer, sizeof(st->cbuffer));
	  if (rbytes <= 0)
	    return (-1);		 
	  if (st->crypto_cb)
	    rbytes = (ssize_t)(st->crypto_cb)(&st->crypto_ctx, st->cbuffer, st->cbuffer, (size_t)rbytes);
	  st->remaining      -= (size_t)rbytes;
	  st->flate.next_in  = (Bytef *)st->cbuffer;
	  st->flate.avail_in = (uInt)rbytes;
	}
	if ((status = inflate(&(st->flate), Z_NO_FLUSH)) < Z_OK)
	{
	  _pdfioFileError(st->pdf, "Unable to decompress stream data: %s", zstrerror(status));
	  return (-1);
	}
	else if (status == Z_STREAM_END)
	  break;
      }
      if (st->flate.avail_out > 0)
        return (-1);			 
      for (; bufptr < bufsecond; remaining --, sptr ++)
	*bufptr++ = *sptr;
      for (; remaining > 0; remaining --, sptr ++, bufptr ++)
	*bufptr = *sptr + bufptr[-(int)pbpixel];
      return ((ssize_t)st->pbsize);
    }
    else
    {
      size_t		pbpixel = st->pbpixel,
      			remaining = st->pbsize - 1;
      unsigned char	*bufptr = (unsigned char *)buffer,
			*bufsecond = (unsigned char *)buffer + pbpixel,
			*sptr = st->psbuffer + 1,
			*pptr = st->prbuffer;
      PDFIO_DEBUG("stream_read: PNG predictor.\n");
      if (bytes < (st->pbsize - 1))
      {
        _pdfioFileError(st->pdf, "Read buffer too small for stream.");
        return (-1);
      }
      st->flate.next_out  = (Bytef *)sptr - 1;
      st->flate.avail_out = (uInt)st->pbsize;
      while (st->flate.avail_out > 0)
      {
	if (st->flate.avail_in == 0)
	{
	  if (sizeof(st->cbuffer) > st->remaining)
	    rbytes = _pdfioFileRead(st->pdf, st->cbuffer, st->remaining);
	  else
	    rbytes = _pdfioFileRead(st->pdf, st->cbuffer, sizeof(st->cbuffer));
	  if (rbytes <= 0)
	    return (-1);		 
	  if (st->crypto_cb)
	    rbytes = (ssize_t)(st->crypto_cb)(&st->crypto_ctx, st->cbuffer, st->cbuffer, (size_t)rbytes);
	  st->remaining      -= (size_t)rbytes;
	  st->flate.next_in  = (Bytef *)st->cbuffer;
	  st->flate.avail_in = (uInt)rbytes;
	}
	if ((status = inflate(&(st->flate), Z_NO_FLUSH)) < Z_OK)
	{
	  _pdfioFileError(st->pdf, "Unable to decompress stream data: %s", zstrerror(status));
	  return (-1);
	}
	else if (status == Z_STREAM_END)
	  break;
      }
      if (st->flate.avail_out > 0)
      {
        PDFIO_DEBUG("stream_read: Early EOF (remaining=%u, avail_in=%d, avail_out=%d, data_type=%d, next_in=<%02X%02X%02X%02X...>).\n", (unsigned)st->remaining, st->flate.avail_in, st->flate.avail_out, st->flate.data_type, st->flate.next_in[0], st->flate.next_in[1], st->flate.next_in[2], st->flate.next_in[3]);
        return (-1);
      }
      PDFIO_DEBUG("stream_read: Line %02X %02X %02X %02X %02X.\n", sptr[-1], sptr[0], sptr[0], sptr[2], sptr[3]);
      switch (sptr[-1])
      {
        case 0 :  
        case 10 :  
            memcpy(buffer, sptr, remaining);
            break;
        case 1 :  
        case 11 :  
            for (; bufptr < bufsecond; remaining --, sptr ++)
              *bufptr++ = *sptr;
            for (; remaining > 0; remaining --, sptr ++, bufptr ++)
              *bufptr = *sptr + bufptr[-(int)pbpixel];
            break;
        case 2 :  
        case 12 :  
            for (; remaining > 0; remaining --, sptr ++, pptr ++)
              *bufptr++ = *sptr + *pptr;
            break;
        case 3 :  
        case 13 :  
	    for (; bufptr < bufsecond; remaining --, sptr ++, pptr ++)
	      *bufptr++ = *sptr + *pptr / 2;
	    for (; remaining > 0; remaining --, sptr ++, pptr ++, bufptr ++)
	      *bufptr = *sptr + (bufptr[-(int)pbpixel] + *pptr) / 2;
            break;
        case 4 :  
        case 14 :  
            for (; bufptr < bufsecond; remaining --, sptr ++, pptr ++)
              *bufptr++ = *sptr + stream_paeth(0, *pptr, 0);
            for (; remaining > 0; remaining --, sptr ++, pptr ++, bufptr ++)
              *bufptr = *sptr + stream_paeth(bufptr[-(int)pbpixel], *pptr, pptr[-(int)pbpixel]);
            break;
        default :
            _pdfioFileError(st->pdf, "Bad PNG filter %d in data stream.", sptr[-1]);
            return (-1);
      }
      memcpy(st->prbuffer, buffer, st->pbsize - 1);
      return ((ssize_t)(st->pbsize - 1));
    }
  }
  return (-1);
}