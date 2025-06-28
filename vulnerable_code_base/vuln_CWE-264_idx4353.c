int lxc_attach(const char* name, const char* lxcpath, lxc_attach_exec_t exec_function, void* exec_payload, lxc_attach_options_t* options, pid_t* attached_process)
{
	int ret, status;
	pid_t init_pid, pid, attached_pid, expected;
	struct lxc_proc_context_info *init_ctx;
	char* cwd;
	char* new_cwd;
	int ipc_sockets[2];
	int procfd;
	signed long personality;
	if (!options)
		options = &attach_static_default_options;
	init_pid = lxc_cmd_get_init_pid(name, lxcpath);
	if (init_pid < 0) {
		ERROR("failed to get the init pid");
		return -1;
	}
	init_ctx = lxc_proc_get_context_info(init_pid);
	if (!init_ctx) {
		ERROR("failed to get context of the init process, pid = %ld", (long)init_pid);
		return -1;
	}
	personality = get_personality(name, lxcpath);
	if (init_ctx->personality < 0) {
		ERROR("Failed to get personality of the container");
		lxc_proc_put_context_info(init_ctx);
		return -1;
	}
	init_ctx->personality = personality;
	init_ctx->container = lxc_container_new(name, lxcpath);
	if (!init_ctx->container)
		return -1;
	if (!fetch_seccomp(init_ctx->container, options))
		WARN("Failed to get seccomp policy");
	if (!no_new_privs(init_ctx->container, options))
		WARN("Could not determine whether PR_SET_NO_NEW_PRIVS is set.");
	cwd = getcwd(NULL, 0);
	if (options->namespaces == -1) {
		options->namespaces = lxc_cmd_get_clone_flags(name, lxcpath);
		if (options->namespaces == -1) {
			ERROR("failed to automatically determine the "
			      "namespaces which the container unshared");
			free(cwd);
			lxc_proc_put_context_info(init_ctx);
			return -1;
		}
	}
	ret = socketpair(PF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0, ipc_sockets);
	if (ret < 0) {
		SYSERROR("could not set up required IPC mechanism for attaching");
		free(cwd);
		lxc_proc_put_context_info(init_ctx);
		return -1;
	}
	pid = fork();
	if (pid < 0) {
		SYSERROR("failed to create first subprocess");
		free(cwd);
		lxc_proc_put_context_info(init_ctx);
		return -1;
	}
	if (pid) {
		pid_t to_cleanup_pid = pid;
		close(ipc_sockets[1]);
		free(cwd);
		if (options->attach_flags & LXC_ATTACH_MOVE_TO_CGROUP) {
			if (!cgroup_attach(name, lxcpath, pid))
				goto cleanup_error;
		}
		status = 0;
		ret = lxc_write_nointr(ipc_sockets[0], &status, sizeof(status));
		if (ret <= 0) {
			ERROR("error using IPC to notify attached process for initialization (0)");
			goto cleanup_error;
		}
		ret = lxc_read_nointr_expect(ipc_sockets[0], &attached_pid, sizeof(attached_pid), NULL);
		if (ret <= 0) {
			if (ret != 0)
				ERROR("error using IPC to receive pid of attached process");
			goto cleanup_error;
		}
		if (options->stdin_fd == 0) {
			signal(SIGINT, SIG_IGN);
			signal(SIGQUIT, SIG_IGN);
		}
		ret = wait_for_pid(pid);
		if (ret < 0)
			goto cleanup_error;
		to_cleanup_pid = attached_pid;
		status = 0;
		ret = lxc_write_nointr(ipc_sockets[0], &status, sizeof(status));
		if (ret <= 0) {
			ERROR("error using IPC to notify attached process for initialization (0)");
			goto cleanup_error;
		}
		expected = 1;
		ret = lxc_read_nointr_expect(ipc_sockets[0], &status, sizeof(status), &expected);
		if (ret <= 0) {
			if (ret != 0)
				ERROR("error using IPC to receive notification from attached process (1)");
			goto cleanup_error;
		}
		status = 2;
		ret = lxc_write_nointr(ipc_sockets[0], &status, sizeof(status));
		if (ret <= 0) {
			ERROR("error using IPC to notify attached process for initialization (2)");
			goto cleanup_error;
		}
		shutdown(ipc_sockets[0], SHUT_RDWR);
		close(ipc_sockets[0]);
		lxc_proc_put_context_info(init_ctx);
		*attached_process = attached_pid;
		return 0;
	cleanup_error:
		shutdown(ipc_sockets[0], SHUT_RDWR);
		close(ipc_sockets[0]);
		if (to_cleanup_pid)
			(void) wait_for_pid(to_cleanup_pid);
		lxc_proc_put_context_info(init_ctx);
		return -1;
	}
	close(ipc_sockets[0]);
	expected = 0;
	status = -1;
	ret = lxc_read_nointr_expect(ipc_sockets[1], &status, sizeof(status), &expected);
	if (ret <= 0) {
		ERROR("error communicating with child process");
		shutdown(ipc_sockets[1], SHUT_RDWR);
		rexit(-1);
	}
	if ((options->attach_flags & LXC_ATTACH_MOVE_TO_CGROUP) && cgns_supported())
		options->namespaces |= CLONE_NEWCGROUP;
	procfd = open("/proc", O_DIRECTORY | O_RDONLY);
	if (procfd < 0) {
		SYSERROR("Unable to open /proc");
		shutdown(ipc_sockets[1], SHUT_RDWR);
		rexit(-1);
	}
	ret = lxc_attach_to_ns(init_pid, options->namespaces);
	if (ret < 0) {
		ERROR("failed to enter the namespace");
		shutdown(ipc_sockets[1], SHUT_RDWR);
		rexit(-1);
	}
	if (options->initial_cwd)
		new_cwd = options->initial_cwd;
	else
		new_cwd = cwd;
	ret = chdir(new_cwd);
	if (ret < 0)
		WARN("could not change directory to '%s'", new_cwd);
	free(cwd);
	{
		struct attach_clone_payload payload = {
			.ipc_socket = ipc_sockets[1],
			.options = options,
			.init_ctx = init_ctx,
			.exec_function = exec_function,
			.exec_payload = exec_payload,
			.procfd = procfd
		};
		pid = lxc_clone(attach_child_main, &payload, CLONE_PARENT);
	}
	if (pid <= 0) {
		SYSERROR("failed to create subprocess");
		shutdown(ipc_sockets[1], SHUT_RDWR);
		rexit(-1);
	}
	ret = lxc_write_nointr(ipc_sockets[1], &pid, sizeof(pid));
	if (ret != sizeof(pid)) {
		ERROR("error using IPC to notify main process of pid of the attached process");
		shutdown(ipc_sockets[1], SHUT_RDWR);
		rexit(-1);
	}
	rexit(0);
}