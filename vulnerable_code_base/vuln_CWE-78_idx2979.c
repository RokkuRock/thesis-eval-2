sigterm_handler(int sig)		 
{
  (void)sig;
  fprintf(stderr,
	  "DEBUG: beh: Job canceled.\n");
  if (job_canceled)
    _exit(CUPS_BACKEND_OK);
  else
    job_canceled = 1;
}