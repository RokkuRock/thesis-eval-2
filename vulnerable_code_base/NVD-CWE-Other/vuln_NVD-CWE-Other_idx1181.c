int main(int argc, char **argv)
{
	int opt;
	uid_t uid = geteuid();
	gid_t gid = getgid();
	mode_t mode = 0;
	struct passwd *pw = NULL;
	struct group *gr = NULL;
	inode_t type = inode_unknown;
	int retval = EXIT_SUCCESS;
	bool trunc = false;
	bool chowner = false;
	bool symlinks = false;
	bool writable = false;
	bool selinux_on = false;
	applet = basename_c(argv[0]);
	while ((opt = getopt_long(argc, argv, getoptstring,
		    longopts, (int *) 0)) != -1)
	{
		switch (opt) {
		case 'D':
			trunc = true;
		case 'd':
			type = inode_dir;
			break;
		case 'F':
			trunc = true;
		case 'f':
			type = inode_file;
			break;
		case 'p':
			type = inode_fifo;
			break;
		case 'm':
			if (parse_mode(&mode, optarg) != 0)
				eerrorx("%s: invalid mode `%s'",
				    applet, optarg);
			break;
		case 'o':
			chowner = true;
			if (parse_owner(&pw, &gr, optarg) != 0)
				eerrorx("%s: owner `%s' not found",
				    applet, optarg);
			break;
		case 's':
#ifndef O_PATH
			symlinks = true;
#endif
			break;
		case 'W':
			writable = true;
			break;
		case_RC_COMMON_GETOPT
		}
	}
	if (optind >= argc)
		usage(EXIT_FAILURE);
	if (writable && type != inode_unknown)
		eerrorx("%s: -W cannot be specified along with -d, -f or -p", applet);
	if (pw) {
		uid = pw->pw_uid;
		gid = pw->pw_gid;
	}
	if (gr)
		gid = gr->gr_gid;
	if (selinux_util_open() == 1)
		selinux_on = true;
	while (optind < argc) {
		if (writable)
			exit(!is_writable(argv[optind]));
		if (do_check(argv[optind], uid, gid, mode, type, trunc, chowner,
					symlinks, selinux_on))
			retval = EXIT_FAILURE;
		optind++;
	}
	if (selinux_on)
		selinux_util_close();
	return retval;
}