vsyslog(pri, fmt, ap)
	int pri;
	register const char *fmt;
	va_list ap;
{
	struct tm now_tm;
	time_t now;
	int fd;
	FILE *f;
	char *buf = 0;
	size_t bufsize = 0;
	size_t prioff, msgoff;
 	struct sigaction action, oldaction;
	struct sigaction *oldaction_ptr = NULL;
 	int sigpipe;
	int saved_errno = errno;
#define	INTERNALLOG	LOG_ERR|LOG_CONS|LOG_PERROR|LOG_PID
	if (pri & ~(LOG_PRIMASK|LOG_FACMASK)) {
		syslog(INTERNALLOG,
		    "syslog: unknown facility/priority: %x", pri);
		pri &= LOG_PRIMASK|LOG_FACMASK;
	}
	if ((LOG_MASK (LOG_PRI (pri)) & LogMask) == 0)
		return;
	if ((pri & LOG_FACMASK) == 0)
		pri |= LogFacility;
	f = open_memstream (&buf, &bufsize);
	prioff = fprintf (f, "<%d>", pri);
	(void) time (&now);
#ifdef USE_IN_LIBIO
        f->_IO_write_ptr += strftime (f->_IO_write_ptr,
                                      f->_IO_write_end - f->_IO_write_ptr,
                                      "%h %e %T ",
				      __localtime_r (&now, &now_tm));
#else
	f->__bufp += strftime (f->__bufp, f->__put_limit - f->__bufp,
			       "%h %e %T ", __localtime_r (&now, &now_tm));
#endif
	msgoff = ftell (f);
	if (LogTag == NULL)
	  LogTag = __progname;
	if (LogTag != NULL)
	  fputs_unlocked (LogTag, f);
	if (LogStat & LOG_PID)
	  fprintf (f, "[%d]", __getpid ());
	if (LogTag != NULL)
	  putc_unlocked (':', f), putc_unlocked (' ', f);
	__set_errno (saved_errno);
	vfprintf (f, fmt, ap);
	fclose (f);
	if (LogStat & LOG_PERROR) {
		struct iovec iov[2];
		register struct iovec *v = iov;
		v->iov_base = buf + msgoff;
		v->iov_len = bufsize - msgoff;
		++v;
		v->iov_base = (char *) "\n";
		v->iov_len = 1;
		(void)__writev(STDERR_FILENO, iov, 2);
	}
	__libc_cleanup_region_start ((void (*) (void *)) cancel_handler,
				     &oldaction_ptr);
	__libc_lock_lock (syslog_lock);
 	memset (&action, 0, sizeof (action));
 	action.sa_handler = sigpipe_handler;
 	sigemptyset (&action.sa_mask);
 	sigpipe = __sigaction (SIGPIPE, &action, &oldaction);
	if (sigpipe == 0)
	  oldaction_ptr = &oldaction;
	if (!connected)
		openlog_internal(LogTag, LogStat | LOG_NDELAY, 0);
	if (LogType == SOCK_STREAM)
	  ++bufsize;
	if (!connected || __send(LogFile, buf, bufsize, 0) < 0)
	  {
	    closelog_internal ();	 
	    if (LogStat & LOG_CONS &&
		(fd = __open(_PATH_CONSOLE, O_WRONLY|O_NOCTTY, 0)) >= 0)
	      {
		dprintf (fd, "%s\r\n", buf + msgoff);
		(void)__close(fd);
	      }
	  }
	if (sigpipe == 0)
		__sigaction (SIGPIPE, &oldaction, (struct sigaction *) NULL);
	__libc_cleanup_region_end (0);
	__libc_lock_unlock (syslog_lock);
	free (buf);
}