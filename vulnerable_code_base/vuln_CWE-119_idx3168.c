test_compressed_stream_overflow (xd3_stream *stream, int ignore)
{
  int ret;
  int i;
  uint8_t *buf;
  if ((buf = (uint8_t*) malloc (TWO_MEGS_AND_DELTA)) == NULL) { return ENOMEM; }
  memset (buf, 0, TWO_MEGS_AND_DELTA);
  for (i = 0; i < (2 << 20); i += 256) 
    {
      int j;
      int off = mt_random(& static_mtrand) % 10;
      for (j = 0; j < 256; j++) 
	{
	  buf[i + j] = j + off;
	}
    }
  if (SIZEOF_XOFF_T == 4)
    {
      ret = test_streaming (stream, buf, buf + (1 << 20), buf + (2 << 20), (1 << 12) + 1);
      if (ret == XD3_INVALID_INPUT && MSG_IS ("decoder file offset overflow"))
	{
	  ret = 0;
	}
      else
	{
          XPR(NT XD3_LIB_ERRMSG (stream, ret));
	  stream->msg = "expected overflow condition";
	  ret = XD3_INTERNAL;
	  goto fail;
	}
    }
  if ((ret = test_streaming (stream, 
			     buf, 
			     buf + (1 << 20), 
			     buf + (2 << 20), 
			     1 << 12))) 
    {
      goto fail;
    }
 fail:
  free (buf);
  return ret;
}