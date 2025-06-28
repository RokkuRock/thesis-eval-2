test_string_matching (xd3_stream *stream, int ignore)
{
  usize_t i;
  int ret;
  xd3_config config;
  char rbuf[TESTBUFSIZE];
  for (i = 0; i < SIZEOF_ARRAY (match_tests); i += 1)
    {
      const string_match_test *test = & match_tests[i];
      char *rptr = rbuf;
      usize_t len = (usize_t) strlen (test->input);
      xd3_free_stream (stream);
      xd3_init_config (& config, 0);
      config.smatch_cfg   = XD3_SMATCH_SOFT;
      config.smatcher_soft.large_look   = 4;
      config.smatcher_soft.large_step   = 4;
      config.smatcher_soft.small_look   = 4;
      config.smatcher_soft.small_chain  = 10;
      config.smatcher_soft.small_lchain = 10;
      config.smatcher_soft.max_lazy     = (test->flags & SM_LAZY) ? 10 : 0;
      config.smatcher_soft.long_enough  = 10;
      if ((ret = xd3_config_stream (stream, & config))) { return ret; }
      if ((ret = xd3_encode_init_full (stream))) { return ret; }
      xd3_avail_input (stream, (uint8_t*)test->input, len);
      if ((ret = stream->smatcher.string_match (stream))) { return ret; }
      *rptr = 0;
      while (! xd3_rlist_empty (& stream->iopt_used))
	{
	  xd3_rinst *inst = xd3_rlist_pop_front (& stream->iopt_used);
	  switch (inst->type)
	    {
	    case XD3_RUN: *rptr++ = 'R'; break;
	    case XD3_CPY: *rptr++ = 'C'; break;
	    default: CHECK(0);
	    }
	  snprintf_func (rptr, rbuf+TESTBUFSIZE-rptr, "%d/%d", 
			 inst->pos, inst->size);
	  rptr += strlen (rptr);
	  if (inst->type == XD3_CPY)
	    {
	      *rptr++ = '@';
	      snprintf_func (rptr, rbuf+TESTBUFSIZE-rptr, "%"Q"d", inst->addr);
	      rptr += strlen (rptr);
	    }
	  *rptr++ = ' ';
	  xd3_rlist_push_back (& stream->iopt_free, inst);
	}
      if (rptr != rbuf)
	{
	  rptr -= 1; *rptr = 0;
	}
      if (strcmp (rbuf, test->result) != 0)
	{
	  XPR(NT "test %u: expected %s: got %s", i, test->result, rbuf);
	  stream->msg = "wrong result";
	  return XD3_INTERNAL;
	}
    }
  return 0;
}