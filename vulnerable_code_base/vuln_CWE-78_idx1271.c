call_backend(char *uri,                  
	     int  argc,                  
	     char **argv,		 
	     char *filename)             
{
  const char	*cups_serverbin;	 
  char		scheme[1024],            
                *ptr,			 
		cmdline[65536];		 
  int           retval;
  strncpy(scheme, uri, sizeof(scheme) - 1);
  if (strlen(uri) > 1023)
    scheme[1023] = '\0';
  if ((ptr = strchr(scheme, ':')) != NULL)
    *ptr = '\0';
  if ((cups_serverbin = getenv("CUPS_SERVERBIN")) == NULL)
    cups_serverbin = CUPS_SERVERBIN;
  if (!strncasecmp(uri, "file:", 5) || uri[0] == '/')
  {
    fprintf(stderr,
	    "ERROR: beh: Direct output into a file not supported.\n");
    exit (CUPS_BACKEND_FAILED);
  }
  else
    snprintf(cmdline, sizeof(cmdline),
	     "%s/backend/%s '%s' '%s' '%s' '%s' '%s' %s",
	     cups_serverbin, scheme, argv[1], argv[2], argv[3],
	     (argc == 6 ? "1" : argv[4]),
	     argv[5], filename);
  setenv("DEVICE_URI", uri, 1);
  fprintf(stderr,
	  "DEBUG: beh: Executing backend command line \"%s\"...\n",
	  cmdline);
  fprintf(stderr,
	  "DEBUG: beh: Using device URI: %s\n",
	  uri);
  retval = system(cmdline) >> 8;
  if (retval == -1)
    fprintf(stderr, "ERROR: Unable to execute backend command line: %s\n",
	    strerror(errno));
  return (retval);
}