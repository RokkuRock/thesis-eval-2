main (int    argc,
      char **argv)
{
  mode_t old_umask;
  cleanup_free char *base_path = NULL;
  int clone_flags;
  char *old_cwd = NULL;
  pid_t pid;
  int event_fd = -1;
  int child_wait_fd = -1;
  int setup_finished_pipe[] = {-1, -1};
  const char *new_cwd;
  uid_t ns_uid;
  gid_t ns_gid;
  struct stat sbuf;
  uint64_t val;
  int res UNUSED;
  cleanup_free char *seccomp_data = NULL;
  size_t seccomp_len;
  struct sock_fprog seccomp_prog;
  cleanup_free char *args_data = NULL;
  if (argc == 2 && (strcmp (argv[1], "--version") == 0))
    print_version_and_exit ();
  real_uid = getuid ();
  real_gid = getgid ();
  acquire_privs ();
  if (prctl (PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0) < 0)
    die_with_error ("prctl(PR_SET_NO_NEW_CAPS) failed");
  read_overflowids ();
  argv0 = argv[0];
  if (isatty (1))
    host_tty_dev = ttyname (1);
  argv++;
  argc--;
  if (argc == 0)
    usage (EXIT_FAILURE, stderr);
  parse_args (&argc, (const char ***) &argv);
  args_data = opt_args_data;
  opt_args_data = NULL;
  if ((requested_caps[0] || requested_caps[1]) && is_privileged)
    die ("--cap-add in setuid mode can be used only by root");
  if (opt_userns_block_fd != -1 && !opt_unshare_user)
    die ("--userns-block-fd requires --unshare-user");
  if (opt_userns_block_fd != -1 && opt_info_fd == -1)
    die ("--userns-block-fd requires --info-fd");
  if (!is_privileged && getuid () != 0)
    opt_unshare_user = TRUE;
#ifdef ENABLE_REQUIRE_USERNS
  if (is_privileged && getuid () != 0)
    opt_unshare_user = TRUE;
#endif
  if (opt_unshare_user_try &&
      stat ("/proc/self/ns/user", &sbuf) == 0)
    {
      bool disabled = FALSE;
      if (stat ("/sys/module/user_namespace/parameters/enable", &sbuf) == 0)
        {
          cleanup_free char *enable = NULL;
          enable = load_file_at (AT_FDCWD, "/sys/module/user_namespace/parameters/enable");
          if (enable != NULL && enable[0] == 'N')
            disabled = TRUE;
        }
      if (stat ("/proc/sys/user/max_user_namespaces", &sbuf) == 0)
        {
          cleanup_free char *max_user_ns = NULL;
          max_user_ns = load_file_at (AT_FDCWD, "/proc/sys/user/max_user_namespaces");
          if (max_user_ns != NULL && strcmp(max_user_ns, "0\n") == 0)
            disabled = TRUE;
        }
      if (!disabled)
        opt_unshare_user = TRUE;
    }
  if (argc == 0)
    usage (EXIT_FAILURE, stderr);
  __debug__ (("Creating root mount point\n"));
  if (opt_sandbox_uid == -1)
    opt_sandbox_uid = real_uid;
  if (opt_sandbox_gid == -1)
    opt_sandbox_gid = real_gid;
  if (!opt_unshare_user && opt_sandbox_uid != real_uid)
    die ("Specifying --uid requires --unshare-user");
  if (!opt_unshare_user && opt_sandbox_gid != real_gid)
    die ("Specifying --gid requires --unshare-user");
  if (!opt_unshare_uts && opt_sandbox_hostname != NULL)
    die ("Specifying --hostname requires --unshare-uts");
  if (opt_as_pid_1 && !opt_unshare_pid)
    die ("Specifying --as-pid-1 requires --unshare-pid");
  if (opt_as_pid_1 && lock_files != NULL)
    die ("Specifying --as-pid-1 and --lock-file is not permitted");
  proc_fd = open ("/proc", O_PATH);
  if (proc_fd == -1)
    die_with_error ("Can't open /proc");
  base_path = xasprintf ("/run/user/%d/.bubblewrap", real_uid);
  if (ensure_dir (base_path, 0755))
    {
      free (base_path);
      base_path = xasprintf ("/tmp/.bubblewrap-%d", real_uid);
      if (ensure_dir (base_path, 0755))
        die_with_error ("Creating root mountpoint failed");
    }
  __debug__ (("creating new namespace\n"));
  if (opt_unshare_pid && !opt_as_pid_1)
    {
      event_fd = eventfd (0, EFD_CLOEXEC | EFD_NONBLOCK);
      if (event_fd == -1)
        die_with_error ("eventfd()");
    }
  block_sigchild ();
  clone_flags = SIGCHLD | CLONE_NEWNS;
  if (opt_unshare_user)
    clone_flags |= CLONE_NEWUSER;
  if (opt_unshare_pid)
    clone_flags |= CLONE_NEWPID;
  if (opt_unshare_net)
    clone_flags |= CLONE_NEWNET;
  if (opt_unshare_ipc)
    clone_flags |= CLONE_NEWIPC;
  if (opt_unshare_uts)
    clone_flags |= CLONE_NEWUTS;
  if (opt_unshare_cgroup)
    {
      if (stat ("/proc/self/ns/cgroup", &sbuf))
        {
          if (errno == ENOENT)
            die ("Cannot create new cgroup namespace because the kernel does not support it");
          else
            die_with_error ("stat on /proc/self/ns/cgroup failed");
        }
      clone_flags |= CLONE_NEWCGROUP;
    }
  if (opt_unshare_cgroup_try)
    if (!stat ("/proc/self/ns/cgroup", &sbuf))
      clone_flags |= CLONE_NEWCGROUP;
  child_wait_fd = eventfd (0, EFD_CLOEXEC);
  if (child_wait_fd == -1)
    die_with_error ("eventfd()");
  if (opt_json_status_fd != -1)
    {
      int ret;
      ret = pipe2 (setup_finished_pipe, O_CLOEXEC);
      if (ret == -1)
        die_with_error ("pipe2()");
    }
  pid = raw_clone (clone_flags, NULL);
  if (pid == -1)
    {
      if (opt_unshare_user)
        {
          if (errno == EINVAL)
            die ("Creating new namespace failed, likely because the kernel does not support user namespaces.  bwrap must be installed setuid on such systems.");
          else if (errno == EPERM && !is_privileged)
            die ("No permissions to creating new namespace, likely because the kernel does not allow non-privileged user namespaces. On e.g. debian this can be enabled with 'sysctl kernel.unprivileged_userns_clone=1'.");
        }
      die_with_error ("Creating new namespace failed");
    }
  ns_uid = opt_sandbox_uid;
  ns_gid = opt_sandbox_gid;
  if (pid != 0)
    {
      if (is_privileged && opt_unshare_user && opt_userns_block_fd == -1)
        {
          write_uid_gid_map (ns_uid, real_uid,
                             ns_gid, real_gid,
                             pid, TRUE, opt_needs_devpts);
        }
      drop_privs (FALSE);
      handle_die_with_parent ();
      if (opt_info_fd != -1)
        {
          cleanup_free char *output = xasprintf ("{\n    \"child-pid\": %i\n}\n", pid);
          dump_info (opt_info_fd, output, TRUE);
          close (opt_info_fd);
        }
      if (opt_json_status_fd != -1)
        {
          cleanup_free char *output = xasprintf ("{ \"child-pid\": %i }\n", pid);
          dump_info (opt_json_status_fd, output, TRUE);
        }
      if (opt_userns_block_fd != -1)
        {
          char b[1];
          (void) TEMP_FAILURE_RETRY (read (opt_userns_block_fd, b, 1));
          close (opt_userns_block_fd);
        }
      val = 1;
      res = write (child_wait_fd, &val, 8);
      close (child_wait_fd);
      return monitor_child (event_fd, pid, setup_finished_pipe[0]);
    }
  if (opt_info_fd != -1)
    close (opt_info_fd);
  if (opt_json_status_fd != -1)
    close (opt_json_status_fd);
  res = read (child_wait_fd, &val, 8);
  close (child_wait_fd);
  switch_to_user_with_privs ();
  if (opt_unshare_net)
    loopback_setup ();  
  ns_uid = opt_sandbox_uid;
  ns_gid = opt_sandbox_gid;
  if (!is_privileged && opt_unshare_user && opt_userns_block_fd == -1)
    {
      if (opt_needs_devpts)
        {
          ns_uid = 0;
          ns_gid = 0;
        }
      write_uid_gid_map (ns_uid, real_uid,
                         ns_gid, real_gid,
                         -1, TRUE, FALSE);
    }
  old_umask = umask (0);
  resolve_symlinks_in_ops ();
  if (mount (NULL, "/", NULL, MS_SLAVE | MS_REC, NULL) < 0)
    die_with_error ("Failed to make / slave");
  if (mount ("tmpfs", base_path, "tmpfs", MS_NODEV | MS_NOSUID, NULL) != 0)
    die_with_error ("Failed to mount tmpfs");
  old_cwd = get_current_dir_name ();
  if (chdir (base_path) != 0)
    die_with_error ("chdir base_path");
  if (mkdir ("newroot", 0755))
    die_with_error ("Creating newroot failed");
  if (mount ("newroot", "newroot", NULL, MS_MGC_VAL | MS_BIND | MS_REC, NULL) < 0)
    die_with_error ("setting up newroot bind");
  if (mkdir ("oldroot", 0755))
    die_with_error ("Creating oldroot failed");
  if (pivot_root (base_path, "oldroot"))
    die_with_error ("pivot_root");
  if (chdir ("/") != 0)
    die_with_error ("chdir / (base path)");
  if (is_privileged)
    {
      pid_t child;
      int privsep_sockets[2];
      if (socketpair (AF_UNIX, SOCK_SEQPACKET | SOCK_CLOEXEC, 0, privsep_sockets) != 0)
        die_with_error ("Can't create privsep socket");
      child = fork ();
      if (child == -1)
        die_with_error ("Can't fork unprivileged helper");
      if (child == 0)
        {
          drop_privs (FALSE);
          close (privsep_sockets[0]);
          setup_newroot (opt_unshare_pid, privsep_sockets[1]);
          exit (0);
        }
      else
        {
          int status;
          uint32_t buffer[2048];   
          uint32_t op, flags;
          const char *arg1, *arg2;
          cleanup_fd int unpriv_socket = -1;
          unpriv_socket = privsep_sockets[0];
          close (privsep_sockets[1]);
          do
            {
              op = read_priv_sec_op (unpriv_socket, buffer, sizeof (buffer),
                                     &flags, &arg1, &arg2);
              privileged_op (-1, op, flags, arg1, arg2);
              if (write (unpriv_socket, buffer, 1) != 1)
                die ("Can't write to op_socket");
            }
          while (op != PRIV_SEP_OP_DONE);
          waitpid (child, &status, 0);
        }
    }
  else
    {
      setup_newroot (opt_unshare_pid, -1);
    }
  close_ops_fd ();
  if (mount ("oldroot", "oldroot", NULL, MS_REC | MS_PRIVATE, NULL) != 0)
    die_with_error ("Failed to make old root rprivate");
  if (umount2 ("oldroot", MNT_DETACH))
    die_with_error ("unmount old root");
  { cleanup_fd int oldrootfd = open ("/", O_DIRECTORY | O_RDONLY);
    if (oldrootfd < 0)
      die_with_error ("can't open /");
    if (chdir ("/newroot") != 0)
      die_with_error ("chdir /newroot");
    if (pivot_root (".", ".") != 0)
      die_with_error ("pivot_root(/newroot)");
    if (fchdir (oldrootfd) < 0)
      die_with_error ("fchdir to oldroot");
    if (umount2 (".", MNT_DETACH) < 0)
      die_with_error ("umount old root");
    if (chdir ("/") != 0)
      die_with_error ("chdir /");
  }
  if (opt_unshare_user &&
      (ns_uid != opt_sandbox_uid || ns_gid != opt_sandbox_gid) &&
      opt_userns_block_fd == -1)
    {
      if (unshare (CLONE_NEWUSER))
        die_with_error ("unshare user ns");
      write_uid_gid_map (opt_sandbox_uid, ns_uid,
                         opt_sandbox_gid, ns_gid,
                         -1, FALSE, FALSE);
    }
  drop_privs (!is_privileged);
  if (opt_block_fd != -1)
    {
      char b[1];
      (void) TEMP_FAILURE_RETRY (read (opt_block_fd, b, 1));
      close (opt_block_fd);
    }
  if (opt_seccomp_fd != -1)
    {
      seccomp_data = load_file_data (opt_seccomp_fd, &seccomp_len);
      if (seccomp_data == NULL)
        die_with_error ("Can't read seccomp data");
      if (seccomp_len % 8 != 0)
        die ("Invalid seccomp data, must be multiple of 8");
      seccomp_prog.len = seccomp_len / 8;
      seccomp_prog.filter = (struct sock_filter *) seccomp_data;
      close (opt_seccomp_fd);
    }
  umask (old_umask);
  new_cwd = "/";
  if (opt_chdir_path)
    {
      if (chdir (opt_chdir_path))
        die_with_error ("Can't chdir to %s", opt_chdir_path);
      new_cwd = opt_chdir_path;
    }
  else if (chdir (old_cwd) == 0)
    {
      new_cwd = old_cwd;
    }
  else
    {
      const char *home = getenv ("HOME");
      if (home != NULL &&
          chdir (home) == 0)
        new_cwd = home;
    }
  xsetenv ("PWD", new_cwd, 1);
  free (old_cwd);
  if (opt_new_session &&
      setsid () == (pid_t) -1)
    die_with_error ("setsid");
  if (label_exec (opt_exec_label) == -1)
    die_with_error ("label_exec %s", argv[0]);
  __debug__ (("forking for child\n"));
  if (!opt_as_pid_1 && (opt_unshare_pid || lock_files != NULL || opt_sync_fd != -1))
    {
      pid = fork ();
      if (pid == -1)
        die_with_error ("Can't fork for pid 1");
      if (pid != 0)
        {
          drop_all_caps (FALSE);
          {
            int dont_close[3];
            int j = 0;
            if (event_fd != -1)
              dont_close[j++] = event_fd;
            if (opt_sync_fd != -1)
              dont_close[j++] = opt_sync_fd;
            dont_close[j++] = -1;
            fdwalk (proc_fd, close_extra_fds, dont_close);
          }
          return do_init (event_fd, pid, seccomp_data != NULL ? &seccomp_prog : NULL);
        }
    }
  __debug__ (("launch executable %s\n", argv[0]));
  if (proc_fd != -1)
    close (proc_fd);
  if (!opt_as_pid_1)
    {
      if (opt_sync_fd != -1)
        close (opt_sync_fd);
    }
  unblock_sigchild ();
  handle_die_with_parent ();
  if (!is_privileged)
    set_ambient_capabilities ();
  if (seccomp_data != NULL &&
      prctl (PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &seccomp_prog) != 0)
    die_with_error ("prctl(PR_SET_SECCOMP)");
  if (setup_finished_pipe[1] != -1)
    {
      char data = 0;
      res = write_to_fd (setup_finished_pipe[1], &data, 1);
    }
  if (execvp (argv[0], argv) == -1)
    {
      if (setup_finished_pipe[1] != -1)
        {
          int saved_errno = errno;
          char data = 0;
          res = write_to_fd (setup_finished_pipe[1], &data, 1);
          errno = saved_errno;
        }
      die_with_error ("execvp %s", argv[0]);
    }
  return 0;
}