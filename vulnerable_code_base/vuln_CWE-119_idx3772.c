test_read_integer_error (xd3_stream *stream, usize_t trunto, const char *msg)
{
  uint64_t eval = 1ULL << 34;
  uint32_t rval;
  xd3_output *buf = NULL;
  const uint8_t *max;
  const uint8_t *inp;
  int ret;
  buf = xd3_alloc_output (stream, buf);
  if ((ret = xd3_emit_uint64_t (stream, & buf, eval)))
    {
      goto fail;
    }
 again:
  inp = buf->base;
  max = buf->base + buf->next - trunto;
  if ((ret = xd3_read_uint32_t (stream, & inp, max, & rval)) != 
      XD3_INVALID_INPUT ||
      !MSG_IS (msg))
    {
      ret = XD3_INTERNAL;
    }
  else if (trunto && trunto < buf->next)
    {
      trunto += 1;
      goto again;
    }
  else
    {
      ret = 0;
    }
 fail:
  xd3_free_output (stream, buf);
  return ret;
}