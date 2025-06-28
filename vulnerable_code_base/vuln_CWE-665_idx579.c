int main(int argc, char **argv) {
	int result;
	int error = FALSE;
	int display_license = FALSE;
	int display_help = FALSE;
	int c = 0;
	struct tm *tm, tm_s;
	time_t now;
	char datestring[256];
	nagios_macros *mac;
	const char *worker_socket = NULL;
	int i;
#ifdef HAVE_SIGACTION
	struct sigaction sig_action;
#endif
#ifdef HAVE_GETOPT_H
	int option_index = 0;
	static struct option long_options[] = {
			{"help", no_argument, 0, 'h'},
			{"version", no_argument, 0, 'V'},
			{"license", no_argument, 0, 'V'},
			{"verify-config", no_argument, 0, 'v'},
			{"daemon", no_argument, 0, 'd'},
			{"test-scheduling", no_argument, 0, 's'},
			{"precache-objects", no_argument, 0, 'p'},
			{"use-precached-objects", no_argument, 0, 'u'},
			{"enable-timing-point", no_argument, 0, 'T'},
			{"worker", required_argument, 0, 'W'},
			{0, 0, 0, 0}
		};
#define getopt(argc, argv, o) getopt_long(argc, argv, o, long_options, &option_index)
#endif
	memset(&loadctl, 0, sizeof(loadctl));
	mac = get_global_macros();
	if(argc < 2)
		error = TRUE;
	while(1) {
		c = getopt(argc, argv, "+hVvdspuxTW");
		if(c == -1 || c == EOF)
			break;
		switch(c) {
			case '?':  
			case 'h':
				display_help = TRUE;
				break;
			case 'V':  
				display_license = TRUE;
				break;
			case 'v':  
				verify_config++;
				break;
			case 's':  
				test_scheduling = TRUE;
				break;
			case 'd':  
				daemon_mode = TRUE;
				break;
			case 'p':  
				precache_objects = TRUE;
				break;
			case 'u':  
				use_precached_objects = TRUE;
				break;
			case 'T':
				enable_timing_point = TRUE;
				break;
			case 'W':
				worker_socket = optarg;
				break;
			case 'x':
				printf("Warning: -x is deprecated and will be removed\n");
				break;
			default:
				break;
			}
		}
#ifdef DEBUG_MEMORY
	mtrace();
#endif
	if(worker_socket) {
		exit(nagios_core_worker(worker_socket));
	}
	init_main_cfg_vars(1);
	init_shared_cfg_vars(1);
	if(daemon_mode == FALSE) {
		printf("\nNagios Core %s\n", PROGRAM_VERSION);
		printf("Copyright (c) 2009-present Nagios Core Development Team and Community Contributors\n");
		printf("Copyright (c) 1999-2009 Ethan Galstad\n");
		printf("Last Modified: %s\n", PROGRAM_MODIFICATION_DATE);
		printf("License: GPL\n\n");
		printf("Website: https://www.nagios.org\n");
		}
	if(display_license == TRUE) {
		printf("This program is free software; you can redistribute it and/or modify\n");
		printf("it under the terms of the GNU General Public License version 2 as\n");
		printf("published by the Free Software Foundation.\n\n");
		printf("This program is distributed in the hope that it will be useful,\n");
		printf("but WITHOUT ANY WARRANTY; without even the implied warranty of\n");
		printf("MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
		printf("GNU General Public License for more details.\n\n");
		printf("You should have received a copy of the GNU General Public License\n");
		printf("along with this program; if not, write to the Free Software\n");
		printf("Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.\n\n");
		exit(OK);
		}
	if(optind >= argc)
		error = TRUE;
	if(error == TRUE || display_help == TRUE) {
		printf("Usage: %s [options] <main_config_file>\n", argv[0]);
		printf("\n");
		printf("Options:\n");
		printf("\n");
		printf("  -v, --verify-config          Verify all configuration data (-v -v for more info)\n");
		printf("  -s, --test-scheduling        Shows projected/recommended check scheduling and other\n");
		printf("                               diagnostic info based on the current configuration files.\n");
		printf("  -T, --enable-timing-point    Enable timed commentary on initialization\n");
		printf("  -x, --dont-verify-paths      Deprecated (Don't check for circular object paths)\n");
		printf("  -p, --precache-objects       Precache object configuration\n");
		printf("  -u, --use-precached-objects  Use precached object config file\n");
		printf("  -d, --daemon                 Starts Nagios in daemon mode, instead of as a foreground process\n");
		printf("  -W, --worker /path/to/socket Act as a worker for an already running daemon\n");
		printf("\n");
		printf("Visit the Nagios website at https://www.nagios.org/ for bug fixes, new\n");
		printf("releases, online documentation, FAQs, information on subscribing to\n");
		printf("the mailing lists, and commercial support options for Nagios.\n");
		printf("\n");
		exit(ERROR);
		}
	config_file = nspath_absolute(argv[optind], NULL);
	if(config_file == NULL) {
		printf("Error allocating memory.\n");
		exit(ERROR);
		}
	config_file_dir = nspath_absolute_dirname(config_file, NULL);
#ifdef HAVE_SIGACTION
	sig_action.sa_sigaction = NULL;
	sig_action.sa_handler = handle_sigxfsz;
	sigfillset(&sig_action.sa_mask);
	sig_action.sa_flags = SA_NODEFER|SA_RESTART;
	sigaction(SIGXFSZ, &sig_action, NULL);
#else
	signal(SIGXFSZ, handle_sigxfsz);
#endif
	if(verify_config || test_scheduling || precache_objects) {
		reset_variables();
		set_loadctl_defaults();
		if(verify_config)
			printf("Reading configuration data...\n");
		result = read_main_config_file(config_file);
		if(result != OK) {
			printf("   Error processing main config file!\n\n");
			exit(EXIT_FAILURE);
			}
		if(verify_config)
			printf("   Read main config file okay...\n");
		if((result = drop_privileges(nagios_user, nagios_group)) == ERROR) {
			printf("   Failed to drop privileges.  Aborting.");
			exit(EXIT_FAILURE);
			}
		if (!verify_config && test_configured_paths() == ERROR) {
			printf("   One or more path problems detected. Aborting.\n");
			exit(EXIT_FAILURE);
			}
		result = read_all_object_data(config_file);
		if(result != OK) {
			printf("   Error processing object config files!\n\n");
			if(!strstr(config_file, "nagios.cfg")) {
				printf("\n***> The name of the main configuration file looks suspicious...\n");
				printf("\n");
				printf("     Make sure you are specifying the name of the MAIN configuration file on\n");
				printf("     the command line and not the name of another configuration file.  The\n");
				printf("     main configuration file is typically '%s'\n", DEFAULT_CONFIG_FILE);
				}
			printf("\n***> One or more problems was encountered while processing the config files...\n");
			printf("\n");
			printf("     Check your configuration file(s) to ensure that they contain valid\n");
			printf("     directives and data definitions.  If you are upgrading from a previous\n");
			printf("     version of Nagios, you should be aware that some variables/definitions\n");
			printf("     may have been removed or modified in this version.  Make sure to read\n");
			printf("     the HTML documentation regarding the config files, as well as the\n");
			printf("     'Whats New' section to find out what has changed.\n\n");
			exit(EXIT_FAILURE);
			}
		if(verify_config) {
			printf("   Read object config files okay...\n\n");
			printf("Running pre-flight check on configuration data...\n\n");
			}
		result = pre_flight_check();
		if(result != OK) {
			printf("\n***> One or more problems was encountered while running the pre-flight check...\n");
			printf("\n");
			printf("     Check your configuration file(s) to ensure that they contain valid\n");
			printf("     directives and data definitions.  If you are upgrading from a previous\n");
			printf("     version of Nagios, you should be aware that some variables/definitions\n");
			printf("     may have been removed or modified in this version.  Make sure to read\n");
			printf("     the HTML documentation regarding the config files, as well as the\n");
			printf("     'Whats New' section to find out what has changed.\n\n");
			exit(EXIT_FAILURE);
			}
		if(verify_config) {
			printf("\nThings look okay - No serious problems were detected during the pre-flight check\n");
			}
		if(test_scheduling == TRUE) {
			init_event_queue();
			timing_point("Done initializing event queue\n");
			initialize_retention_data(config_file);
			read_initial_state_information();
			timing_point("Retention data and initial state parsed\n");
			init_timing_loop();
			timing_point("Timing loop initialized\n");
			display_scheduling_info();
			}
		if(precache_objects) {
			result = fcache_objects(object_precache_file);
			timing_point("Done precaching objects\n");
			if(result == OK) {
				printf("Object precache file created:\n%s\n", object_precache_file);
				}
			else {
				printf("Failed to precache objects to '%s': %s\n", object_precache_file, strerror(errno));
				}
			}
		cleanup();
		timing_point("Exiting\n");
		neb_free_module_list();
		free(config_file_dir);
		free(config_file);
		exit(result);
		}
	else {
		if (strchr(argv[0], '/'))
			nagios_binary_path = nspath_absolute(argv[0], NULL);
		else
			nagios_binary_path = strdup(argv[0]);
		if (!nagios_binary_path) {
			logit(NSLOG_RUNTIME_ERROR, TRUE, "Error: Unable to allocate memory for nagios_binary_path\n");
			exit(EXIT_FAILURE);
			}
		if (!(nagios_iobs = iobroker_create())) {
			logit(NSLOG_RUNTIME_ERROR, TRUE, "Error: Failed to create IO broker set: %s\n",
				  strerror(errno));
			exit(EXIT_FAILURE);
			}
		do {
			wproc_num_workers_spawned = wproc_num_workers_online = 0;
			caught_signal = sigshutdown = FALSE;
			sig_id = 0;
			reset_variables();
			timing_point("Variables reset\n");
			nagios_pid = (int)getpid();
			result = read_main_config_file(config_file);
			if (result != OK) {
				logit(NSLOG_CONFIG_ERROR, TRUE, "Error: Failed to process config file '%s'. Aborting\n", config_file);
				exit(EXIT_FAILURE);
				}
			timing_point("Main config file read\n");
			program_start = time(NULL);
			my_free(mac->x[MACRO_PROCESSSTARTTIME]);
			asprintf(&mac->x[MACRO_PROCESSSTARTTIME], "%llu", (unsigned long long)program_start);
			if(drop_privileges(nagios_user, nagios_group) == ERROR) {
				logit(NSLOG_PROCESS_INFO | NSLOG_RUNTIME_ERROR | NSLOG_CONFIG_ERROR, TRUE, "Failed to drop privileges.  Aborting.");
				cleanup();
				exit(ERROR);
				}
			if (test_path_access(nagios_binary_path, X_OK)) {
				logit(NSLOG_RUNTIME_ERROR, TRUE, "Error: failed to access() %s: %s\n", nagios_binary_path, strerror(errno));
				logit(NSLOG_RUNTIME_ERROR, TRUE, "Error: Spawning workers will be impossible. Aborting.\n");
				exit(EXIT_FAILURE);
				}
			if (test_configured_paths() == ERROR) {
				exit(EXIT_FAILURE);
				}
			if(daemon_mode == TRUE && sigrestart == FALSE) {
				result = daemon_init();
				if(result == ERROR) {
					logit(NSLOG_PROCESS_INFO | NSLOG_RUNTIME_ERROR, TRUE, "Bailing out due to failure to daemonize. (PID=%d)", (int)getpid());
					cleanup();
					exit(EXIT_FAILURE);
					}
				nagios_pid = (int)getpid();
				}
			logit(NSLOG_PROCESS_INFO, TRUE, "Nagios %s starting... (PID=%d)\n", PROGRAM_VERSION, (int)getpid());
			now = time(NULL);
			tm = localtime_r(&now, &tm_s);
			strftime(datestring, sizeof(datestring), "%a %b %d %H:%M:%S %Z %Y", tm);
			logit(NSLOG_PROCESS_INFO, TRUE, "Local time is %s", datestring);
			write_log_file_info(NULL);
			open_debug_log();
#ifdef USE_EVENT_BROKER
			neb_init_modules();
			neb_init_callback_list();
#endif
			timing_point("NEB module API initialized\n");
			setup_sighandler();
			if (qh_init(qh_socket_path ? qh_socket_path : DEFAULT_QUERY_SOCKET) != OK) {
				logit(NSLOG_RUNTIME_ERROR, TRUE, "Error: Failed to initialize query handler. Aborting\n");
				exit(EXIT_FAILURE);
			}
			timing_point("Query handler initialized\n");
			nerd_init();
			timing_point("NERD initialized\n");
			if(init_workers(num_check_workers) < 0) {
				logit(NSLOG_RUNTIME_ERROR, TRUE, "Failed to spawn workers. Aborting\n");
				exit(EXIT_FAILURE);
			}
			timing_point("%u workers spawned\n", wproc_num_workers_spawned);
			i = 0;
			while (i < 50 && wproc_num_workers_online < wproc_num_workers_spawned) {
				iobroker_poll(nagios_iobs, 50);
				i++;
			}
			timing_point("%u workers connected\n", wproc_num_workers_online);
			set_loadctl_defaults();
#ifdef USE_EVENT_BROKER
			if (neb_load_all_modules() != OK) {
				logit(NSLOG_CONFIG_ERROR, ERROR, "Error: Module loading failed. Aborting.\n");
				if (daemon_dumps_core)
					neb_unload_all_modules(NEBMODULE_FORCE_UNLOAD, NEBMODULE_NEB_SHUTDOWN);
				exit(EXIT_FAILURE);
				}
			timing_point("Modules loaded\n");
			broker_program_state(NEBTYPE_PROCESS_PRELAUNCH, NEBFLAG_NONE, NEBATTR_NONE, NULL);
			timing_point("First callback made\n");
#endif
			if(result == OK)
				result = read_all_object_data(config_file);
			if(result != OK)
				logit(NSLOG_PROCESS_INFO | NSLOG_RUNTIME_ERROR | NSLOG_CONFIG_ERROR, TRUE, "Bailing out due to one or more errors encountered in the configuration files. Run Nagios from the command line with the -v option to verify your config before restarting. (PID=%d)", (int)getpid());
			else {
				if((result = pre_flight_check()) != OK)
					logit(NSLOG_PROCESS_INFO | NSLOG_RUNTIME_ERROR | NSLOG_VERIFICATION_ERROR, TRUE, "Bailing out due to errors encountered while running the pre-flight check.  Run Nagios from the command line with the -v option to verify your config before restarting. (PID=%d)\n", (int)getpid());
				}
			if(result != OK) {
				if(sigrestart == TRUE) {
					cleanup_status_data(TRUE);
					}
#ifdef USE_EVENT_BROKER
				broker_program_state(NEBTYPE_PROCESS_SHUTDOWN, NEBFLAG_PROCESS_INITIATED, NEBATTR_SHUTDOWN_ABNORMAL, NULL);
#endif
				cleanup();
				exit(ERROR);
				}
			timing_point("Object configuration parsed and understood\n");
			fcache_objects(object_cache_file);
			timing_point("Objects cached\n");
			init_event_queue();
			timing_point("Event queue initialized\n");
#ifdef USE_EVENT_BROKER
			broker_program_state(NEBTYPE_PROCESS_START, NEBFLAG_NONE, NEBATTR_NONE, NULL);
#endif
			if(sigrestart == FALSE) {
				initialize_status_data(config_file);
				timing_point("Status data initialized\n");
				}
			initialize_downtime_data();
			timing_point("Downtime data initialized\n");
			initialize_retention_data(config_file);
			timing_point("Retention data initialized\n");
			read_initial_state_information();
			timing_point("Initial state information read\n");
			initialize_comment_data();
			timing_point("Comment data initialized\n");
			initialize_performance_data(config_file);
			timing_point("Performance data initialized\n");
			init_timing_loop();
			timing_point("Event timing loop initialized\n");
			init_check_stats();
			timing_point("check stats initialized\n");
			check_for_nagios_updates(FALSE, TRUE);
			timing_point("Update check concluded\n");
			update_all_status_data();
			timing_point("Status data updated\n");
			log_host_states(INITIAL_STATES, NULL);
			log_service_states(INITIAL_STATES, NULL);
			timing_point("Initial states logged\n");
			sigrestart = FALSE;
			launch_command_file_worker();
			timing_point("Command file worker launched\n");
#ifdef USE_EVENT_BROKER
			broker_program_state(NEBTYPE_PROCESS_EVENTLOOPSTART, NEBFLAG_NONE, NEBATTR_NONE, NULL);
#endif
			event_start = time(NULL);
			my_free(mac->x[MACRO_EVENTSTARTTIME]);
			asprintf(&mac->x[MACRO_EVENTSTARTTIME], "%llu", (unsigned long long)event_start);
			timing_point("Entering event execution loop\n");
			event_execution_loop();
			qh_deinit(qh_socket_path ? qh_socket_path : DEFAULT_QUERY_SOCKET);
			if(caught_signal == TRUE) {
				if(sig_id == SIGHUP)
					logit(NSLOG_PROCESS_INFO, TRUE, "Caught SIGHUP, restarting...\n");
				}
#ifdef USE_EVENT_BROKER
			broker_program_state(NEBTYPE_PROCESS_EVENTLOOPEND, NEBFLAG_NONE, NEBATTR_NONE, NULL);
			if(sigshutdown == TRUE)
				broker_program_state(NEBTYPE_PROCESS_SHUTDOWN, NEBFLAG_USER_INITIATED, NEBATTR_SHUTDOWN_NORMAL, NULL);
			else if(sigrestart == TRUE)
				broker_program_state(NEBTYPE_PROCESS_RESTART, NEBFLAG_USER_INITIATED, NEBATTR_RESTART_NORMAL, NULL);
#endif
			save_state_information(FALSE);
			cleanup_retention_data();
			cleanup_performance_data();
			cleanup_downtime_data();
			if(sigrestart == FALSE) {
				cleanup_status_data(TRUE);
				}
			free_worker_memory(WPROC_FORCE);
			if(sigshutdown == TRUE) {
				iobroker_destroy(nagios_iobs, IOBROKER_CLOSE_SOCKETS);
				nagios_iobs = NULL;
				logit(NSLOG_PROCESS_INFO, TRUE, "Successfully shutdown... (PID=%d)\n", (int)getpid());
				}
			cleanup();
			close_debug_log();
			}
		while(sigrestart == TRUE && sigshutdown == FALSE);
		if(daemon_mode == TRUE)
			unlink(lock_file);
		my_free(lock_file);
		my_free(config_file);
		my_free(config_file_dir);
		my_free(nagios_binary_path);
		}
	return OK;
	}