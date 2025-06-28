test_recode_command2 (xd3_stream *stream, int has_source,
		      int variant, int change)
{
  int has_adler32 = (variant & 0x1) != 0;
  int has_apphead = (variant & 0x2) != 0;
  int has_secondary = (variant & 0x4) != 0;
  int change_adler32 = (change & 0x1) != 0;
  int change_apphead = (change & 0x2) != 0;
  int change_secondary = (change & 0x4) != 0;
  int recoded_adler32 = change_adler32 ? !has_adler32 : has_adler32;
  int recoded_apphead = change_apphead ? !has_apphead : has_apphead;
  int recoded_secondary = change_secondary ? !has_secondary : has_secondary;
  char ecmd[TESTBUFSIZE], recmd[TESTBUFSIZE], dcmd[TESTBUFSIZE];
  xoff_t tsize, ssize;
  int ret;
  test_setup ();
  if ((ret = test_make_inputs (stream, has_source ? & ssize : NULL, & tsize)))
    {
      return ret;
    }
  snprintf_func (ecmd, TESTBUFSIZE, "%s %s -f %s %s %s %s %s %s %s", 
	    program_name, test_softcfg_str,
	    has_adler32 ? "" : "-n ",
	    has_apphead ? "-A=encode_apphead " : "-A= ",
	    has_secondary ? "-S djw " : "-S none ",
	    has_source ? "-s " : "",
	    has_source ? TEST_SOURCE_FILE : "",
	    TEST_TARGET_FILE,
	    TEST_DELTA_FILE);
  if ((ret = system (ecmd)) != 0)
    {
      XPR(NT "encode command: %s\n", ecmd);
      stream->msg = "encode cmd failed";
      return XD3_INTERNAL;
    }
  snprintf_func (recmd, TESTBUFSIZE,
	    "%s recode %s -f %s %s %s %s %s", program_name, test_softcfg_str,
	    recoded_adler32 ? "" : "-n ",
	    !change_apphead ? "" : 
	        (recoded_apphead ? "-A=recode_apphead " : "-A= "),
	    recoded_secondary ? "-S djw " : "-S none ",
	    TEST_DELTA_FILE,
	    TEST_COPY_FILE);
  if ((ret = system (recmd)) != 0)
    {
      XPR(NT "recode command: %s\n", recmd);
      stream->msg = "recode cmd failed";
      return XD3_INTERNAL;
    }
  if ((ret = check_vcdiff_header (stream,
				  TEST_COPY_FILE,
				  "VCDIFF window indicator",
				  "VCD_SOURCE",
				  has_source))) { return ret; }
  if ((ret = check_vcdiff_header (stream,
				  TEST_COPY_FILE,
				  "VCDIFF header indicator",
				  "VCD_SECONDARY",
				  recoded_secondary))) { return ret; }
  if ((ret = check_vcdiff_header (stream,
				  TEST_COPY_FILE,
				  "VCDIFF window indicator",
				  "VCD_ADLER32",
				  has_adler32 && recoded_adler32)))
    {
      return ret;
    }
  if (!change_apphead)
    {
      if ((ret = check_vcdiff_header (stream,
				      TEST_COPY_FILE,
				      "VCDIFF header indicator",
				      "VCD_APPHEADER",
				      has_apphead)))
	{
	  return ret;
	}
      if ((ret = check_vcdiff_header (stream,
				      TEST_COPY_FILE,
				      "VCDIFF application header",
				      "encode_apphead",
				      has_apphead)))
	{
	  return ret;
	}
    }
  else
    {
      if ((ret = check_vcdiff_header (stream,
				      TEST_COPY_FILE,
				      "VCDIFF header indicator",
				      "VCD_APPHEADER",
				      recoded_apphead)))
	{
	  return ret;
	}
      if (recoded_apphead &&
	  (ret = check_vcdiff_header (stream,
				      TEST_COPY_FILE,
				      "VCDIFF application header",
				      "recode_apphead",
				      1)))
	{
	  return ret;
	}
    }
  snprintf_func (dcmd, TESTBUFSIZE, "%s -fd %s %s %s %s ", program_name,
	    has_source ? "-s " : "",
	    has_source ? TEST_SOURCE_FILE : "",
	    TEST_COPY_FILE,
	    TEST_RECON_FILE);
  if ((ret = system (dcmd)) != 0)
    {
      XPR(NT "decode command: %s\n", dcmd);
      stream->msg = "decode cmd failed";
      return XD3_INTERNAL;
    }
  if ((ret = test_compare_files (TEST_TARGET_FILE, TEST_RECON_FILE)))
    {
      return ret;
    }
  return 0;
}