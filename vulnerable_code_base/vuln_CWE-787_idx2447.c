bool initialise_control(rzip_control *control)
{
	time_t now_t, tdiff;
	char localeptr[] = "./", *eptr;  
	size_t len;
	memset(control, 0, sizeof(rzip_control));
	control->msgout = stderr;
	control->msgerr = stderr;
	register_outputfile(control, control->msgout);
	control->flags = FLAG_SHOW_PROGRESS | FLAG_KEEP_FILES | FLAG_THRESHOLD;
	control->suffix = ".lrz";
	control->compression_level = 7;
	control->ramsize = get_ram(control);
	if (unlikely(control->ramsize == -1))
		return false;
	control->threads = PROCESSORS;	 
	control->page_size = PAGE_SIZE;
	control->nice_val = 19;
	if (unlikely((now_t = time(NULL)) == ((time_t)-1)))
		fatal_return(("Failed to call time in main\n"), false);
	if (unlikely(now_t < T_ZERO)) {
		print_output("Warning your time reads before the year 2011, check your system clock\n");
		now_t = T_ZERO;
	}
	tdiff = (now_t - T_ZERO) / 4;
	now_t = T_ZERO + tdiff;
	control->secs = now_t;
	control->encloops = nloops(control->secs, control->salt, control->salt + 1);
	if (unlikely(!get_rand(control, control->salt + 2, 6)))
		return false;
	eptr = getenv("TMPDIR");
	if (!eptr)
		eptr = getenv("TMP");
	if (!eptr)
		eptr = getenv("TEMPDIR");
	if (!eptr)
		eptr = getenv("TEMP");
	if (!eptr)
		eptr = localeptr;
	len = strlen(eptr);
	control->tmpdir = malloc(len + 2);
	if (control->tmpdir == NULL)
		fatal_return(("Failed to allocate for tmpdir\n"), false);
	strcpy(control->tmpdir, eptr);
	if (control->tmpdir[len - 1] != '/') {
		control->tmpdir[len] = '/';  
		control->tmpdir[len + 1] = '\0';
	}
	return true;
}