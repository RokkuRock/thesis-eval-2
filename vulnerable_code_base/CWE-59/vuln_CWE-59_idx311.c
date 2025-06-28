static void setup_private_mount(const char *snap_name)
{
	uid_t uid = getuid();
	gid_t gid = getgid();
	char tmpdir[MAX_BUF] = { 0 };
	sc_must_snprintf(tmpdir, sizeof(tmpdir), "/tmp/snap.%s_XXXXXX", snap_name);
	if (mkdtemp(tmpdir) == NULL) {
		die("cannot create temporary directory essential for private /tmp");
	}
	mode_t old_mask = umask(0);
	char *d = sc_strdup(tmpdir);
	sc_must_snprintf(tmpdir, sizeof(tmpdir), "%s/tmp", d);
	free(d);
	if (mkdir(tmpdir, 01777) != 0) {
		die("cannot create temporary directory for private /tmp");
	}
	umask(old_mask);
	char *pwd = get_current_dir_name();
	if (pwd == NULL)
		die("cannot get current working directory");
	if (chdir("/") != 0)
		die("cannot change directory to '/'");
	sc_do_mount(tmpdir, "/tmp", NULL, MS_BIND, NULL);
	sc_do_mount("none", "/tmp", NULL, MS_PRIVATE, NULL);
	if (chown("/tmp/", uid, gid) < 0) {
		die("cannot change ownership of /tmp");
	}
	if (chdir(pwd) != 0)
		die("cannot change current working directory to the original directory");
	free(pwd);
}