keepalived_main(int argc, char **argv)
{
	bool report_stopped = true;
	struct utsname uname_buf;
	char *end;
	set_time_now();
	save_cmd_line_options(argc, argv);
	debug = 0;
#ifndef _DEBUG_
	prog_type = PROG_TYPE_PARENT;
#endif
#ifdef _WITH_VRRP_
	__set_bit(DAEMON_VRRP, &daemon_mode);
#endif
#ifdef _WITH_LVS_
	__set_bit(DAEMON_CHECKERS, &daemon_mode);
#endif
#ifdef _WITH_BFD_
	__set_bit(DAEMON_BFD, &daemon_mode);
#endif
	openlog(PACKAGE_NAME, LOG_PID, log_facility);
#ifdef _MEM_CHECK_
	mem_log_init(PACKAGE_NAME, "Parent process");
#endif
	if (uname(&uname_buf))
		log_message(LOG_INFO, "Unable to get uname() information - error %d", errno);
	else {
		os_major = (unsigned)strtoul(uname_buf.release, &end, 10);
		if (*end != '.')
			os_major = 0;
		else {
			os_minor = (unsigned)strtoul(end + 1, &end, 10);
			if (*end != '.')
				os_major = 0;
			else {
				if (!isdigit(end[1]))
					os_major = 0;
				else
					os_release = (unsigned)strtoul(end + 1, &end, 10);
			}
		}
		if (!os_major)
			log_message(LOG_INFO, "Unable to parse kernel version %s", uname_buf.release);
		if (!config_id) {
			end = strchrnul(uname_buf.nodename, '.');
			config_id = MALLOC((size_t)(end - uname_buf.nodename) + 1);
			strncpy(config_id, uname_buf.nodename, (size_t)(end - uname_buf.nodename));
			config_id[end - uname_buf.nodename] = '\0';
		}
	}
	if (parse_cmdline(argc, argv)) {
		closelog();
		if (!__test_bit(NO_SYSLOG_BIT, &debug))
			openlog(PACKAGE_NAME, LOG_PID | ((__test_bit(LOG_CONSOLE_BIT, &debug)) ? LOG_CONS : 0) , log_facility);
	}
	if (__test_bit(LOG_CONSOLE_BIT, &debug))
		enable_console_log();
#ifdef GIT_COMMIT
	log_message(LOG_INFO, "Starting %s, git commit %s", version_string, GIT_COMMIT);
#else
	log_message(LOG_INFO, "Starting %s", version_string);
#endif
	core_dump_init();
	if (os_major) {
		if (KERNEL_VERSION(os_major, os_minor, os_release) < LINUX_VERSION_CODE) {
			log_message(LOG_INFO, "WARNING - keepalived was build for newer Linux %d.%d.%d, running on %s %s %s",
					(LINUX_VERSION_CODE >> 16) & 0xff,
					(LINUX_VERSION_CODE >>  8) & 0xff,
					(LINUX_VERSION_CODE      ) & 0xff,
					uname_buf.sysname, uname_buf.release, uname_buf.version);
		} else {
			log_message(LOG_INFO, "Running on %s %s %s (built for Linux %d.%d.%d)",
					uname_buf.sysname, uname_buf.release, uname_buf.version,
					(LINUX_VERSION_CODE >> 16) & 0xff,
					(LINUX_VERSION_CODE >>  8) & 0xff,
					(LINUX_VERSION_CODE      ) & 0xff);
		}
	}
#ifndef _DEBUG_
	log_command_line(0);
#endif
	if (!check_conf_file(conf_file)) {
		if (__test_bit(CONFIG_TEST_BIT, &debug))
			config_test_exit();
		goto end;
	}
	global_data = alloc_global_data();
	read_config_file();
	init_global_data(global_data, NULL);
#if HAVE_DECL_CLONE_NEWNET
	if (override_namespace) {
		if (global_data->network_namespace) {
			log_message(LOG_INFO, "Overriding config net_namespace '%s' with command line namespace '%s'", global_data->network_namespace, override_namespace);
			FREE(global_data->network_namespace);
		}
		global_data->network_namespace = override_namespace;
		override_namespace = NULL;
	}
#endif
	if (!__test_bit(CONFIG_TEST_BIT, &debug) &&
	    (global_data->instance_name
#if HAVE_DECL_CLONE_NEWNET
	     || global_data->network_namespace
#endif
					      )) {
		if ((syslog_ident = make_syslog_ident(PACKAGE_NAME))) {
			log_message(LOG_INFO, "Changing syslog ident to %s", syslog_ident);
			closelog();
			openlog(syslog_ident, LOG_PID | ((__test_bit(LOG_CONSOLE_BIT, &debug)) ? LOG_CONS : 0), log_facility);
		}
		else
			log_message(LOG_INFO, "Unable to change syslog ident");
		use_pid_dir = true;
		open_log_file(log_file_name,
				NULL,
#if HAVE_DECL_CLONE_NEWNET
				global_data->network_namespace,
#else
				NULL,
#endif
				global_data->instance_name);
	}
	set_child_finder_name(find_keepalived_child_name);
	if (!__test_bit(CONFIG_TEST_BIT, &debug)) {
		if (use_pid_dir) {
			create_pid_dir();
		}
	}
#if HAVE_DECL_CLONE_NEWNET
	if (global_data->network_namespace) {
		if (global_data->network_namespace && !set_namespaces(global_data->network_namespace)) {
			log_message(LOG_ERR, "Unable to set network namespace %s - exiting", global_data->network_namespace);
			goto end;
		}
	}
#endif
	if (!__test_bit(CONFIG_TEST_BIT, &debug)) {
		if (global_data->instance_name) {
			if (!main_pidfile && (main_pidfile = make_pidfile_name(KEEPALIVED_PID_DIR KEEPALIVED_PID_FILE, global_data->instance_name, PID_EXTENSION)))
				free_main_pidfile = true;
#ifdef _WITH_LVS_
			if (!checkers_pidfile && (checkers_pidfile = make_pidfile_name(KEEPALIVED_PID_DIR CHECKERS_PID_FILE, global_data->instance_name, PID_EXTENSION)))
				free_checkers_pidfile = true;
#endif
#ifdef _WITH_VRRP_
			if (!vrrp_pidfile && (vrrp_pidfile = make_pidfile_name(KEEPALIVED_PID_DIR VRRP_PID_FILE, global_data->instance_name, PID_EXTENSION)))
				free_vrrp_pidfile = true;
#endif
#ifdef _WITH_BFD_
			if (!bfd_pidfile && (bfd_pidfile = make_pidfile_name(KEEPALIVED_PID_DIR VRRP_PID_FILE, global_data->instance_name, PID_EXTENSION)))
				free_bfd_pidfile = true;
#endif
		}
		if (use_pid_dir) {
			if (!main_pidfile)
				main_pidfile = KEEPALIVED_PID_DIR KEEPALIVED_PID_FILE PID_EXTENSION;
#ifdef _WITH_LVS_
			if (!checkers_pidfile)
				checkers_pidfile = KEEPALIVED_PID_DIR CHECKERS_PID_FILE PID_EXTENSION;
#endif
#ifdef _WITH_VRRP_
			if (!vrrp_pidfile)
				vrrp_pidfile = KEEPALIVED_PID_DIR VRRP_PID_FILE PID_EXTENSION;
#endif
#ifdef _WITH_BFD_
			if (!bfd_pidfile)
				bfd_pidfile = KEEPALIVED_PID_DIR BFD_PID_FILE PID_EXTENSION;
#endif
		}
		else
		{
			if (!main_pidfile)
				main_pidfile = PID_DIR KEEPALIVED_PID_FILE PID_EXTENSION;
#ifdef _WITH_LVS_
			if (!checkers_pidfile)
				checkers_pidfile = PID_DIR CHECKERS_PID_FILE PID_EXTENSION;
#endif
#ifdef _WITH_VRRP_
			if (!vrrp_pidfile)
				vrrp_pidfile = PID_DIR VRRP_PID_FILE PID_EXTENSION;
#endif
#ifdef _WITH_BFD_
			if (!bfd_pidfile)
				bfd_pidfile = PID_DIR BFD_PID_FILE PID_EXTENSION;
#endif
		}
		if (keepalived_running(daemon_mode)) {
			log_message(LOG_INFO, "daemon is already running");
			report_stopped = false;
			goto end;
		}
	}
	if (!__test_bit(DONT_FORK_BIT, &debug) &&
	    xdaemon(false, false, true) > 0) {
		closelog();
		FREE_PTR(config_id);
		FREE_PTR(orig_core_dump_pattern);
		close_std_fd();
		exit(0);
	}
	umask(0);
#ifdef _MEM_CHECK_
	enable_mem_log_termination();
#endif
	if (__test_bit(CONFIG_TEST_BIT, &debug)) {
		validate_config();
		config_test_exit();
	}
	if (!pidfile_write(main_pidfile, getpid()))
		goto end;
	master = thread_make_master();
	signal_init();
	if (!start_keepalived())
		log_message(LOG_INFO, "Warning - keepalived has no configuration to run");
	initialise_debug_options();
#ifdef THREAD_DUMP
	register_parent_thread_addresses();
#endif
	launch_thread_scheduler(master);
	stop_keepalived();
#ifdef THREAD_DUMP
	deregister_thread_addresses();
#endif
end:
	if (report_stopped) {
#ifdef GIT_COMMIT
		log_message(LOG_INFO, "Stopped %s, git commit %s", version_string, GIT_COMMIT);
#else
		log_message(LOG_INFO, "Stopped %s", version_string);
#endif
	}
#if HAVE_DECL_CLONE_NEWNET
	if (global_data && global_data->network_namespace)
		clear_namespaces();
#endif
	if (use_pid_dir)
		remove_pid_dir();
	if (orig_core_dump_pattern)
		update_core_dump_pattern(orig_core_dump_pattern);
	free_parent_mallocs_startup(false);
	free_parent_mallocs_exit();
	free_global_data(global_data);
	closelog();
#ifndef _MEM_CHECK_LOG_
	FREE_PTR(syslog_ident);
#else
	if (syslog_ident)
		free(syslog_ident);
#endif
	close_std_fd();
	exit(KEEPALIVED_EXIT_OK);
}