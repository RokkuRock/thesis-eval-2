test_compare_files (const char* tgt, const char *rec)
{
  FILE *orig, *recons;
  static uint8_t obuf[TESTBUFSIZE], rbuf[TESTBUFSIZE];
  xoff_t offset = 0;
  size_t i;
  size_t oc, rc;
  xoff_t diffs = 0;
  if ((orig = fopen (tgt, "r")) == NULL)
    {
      XPR(NT "open %s failed\n", tgt);
      return get_errno ();
    }
  if ((recons = fopen (rec, "r")) == NULL)
    {
      XPR(NT "open %s failed\n", rec);
      return get_errno ();
    }
  for (;;)
    {
      oc = fread (obuf, 1, TESTBUFSIZE, orig);
      rc = fread (rbuf, 1, TESTBUFSIZE, recons);
      if (oc != rc)
	{
	  return XD3_INTERNAL;
	}
      if (oc == 0)
	{
	  break;
	}
      for (i = 0; i < oc; i += 1)
	{
	  if (obuf[i] != rbuf[i])
 	    {
	      XPR(NT "byte %u (read %u @ %"Q"u) %d != %d\n", 
		  (int)i, (int)oc, offset, obuf[i], rbuf[i]);
	      diffs++;
	      return XD3_INTERNAL;
	    }
	}
      offset += oc;
    }
    fclose (orig);
    fclose (recons);
    if (diffs != 0) 
      {
	return XD3_INTERNAL;
      }
    return 0;
}