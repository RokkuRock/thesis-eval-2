int main(int argc, char** argv)
{
    int fd = xopen("/dev/null", O_RDWR);
    while (fd < 2)
        fd = xdup(fd);
    if (fd > 2)
        close(fd);
    int err = 1;
    logmode = LOGMODE_JOURNAL;
    load_abrt_conf();
    bool setting_MakeCompatCore;
    bool setting_SaveBinaryImage;
    bool setting_SaveFullCore;
    bool setting_CreateCoreBacktrace;
    bool setting_SaveContainerizedPackageData;
    bool setting_StandaloneHook;
    {
        map_string_t *settings = new_map_string();
        load_abrt_plugin_conf_file("CCpp.conf", settings);
        const char *value;
        value = get_map_string_item_or_NULL(settings, "MakeCompatCore");
        setting_MakeCompatCore = value && string_to_bool(value);
        value = get_map_string_item_or_NULL(settings, "SaveBinaryImage");
        setting_SaveBinaryImage = value && string_to_bool(value);
        value = get_map_string_item_or_NULL(settings, "SaveFullCore");
        setting_SaveFullCore = value ? string_to_bool(value) : true;
        value = get_map_string_item_or_NULL(settings, "CreateCoreBacktrace");
        setting_CreateCoreBacktrace = value ? string_to_bool(value) : true;
        value = get_map_string_item_or_NULL(settings, "SaveContainerizedPackageData");
        setting_SaveContainerizedPackageData = value && string_to_bool(value);
        if (!g_settings_explorechroots)
        {
            if (setting_SaveContainerizedPackageData)
                log_warning("Ignoring SaveContainerizedPackageData because ExploreChroots is disabled");
            setting_SaveContainerizedPackageData = false;
        }
        value = get_map_string_item_or_NULL(settings, "StandaloneHook");
        setting_StandaloneHook = value && string_to_bool(value);
        value = get_map_string_item_or_NULL(settings, "VerboseLog");
        if (value)
            g_verbose = xatoi_positive(value);
        free_map_string(settings);
    }
    if (argc == 2 && strcmp(argv[1], "--config-test"))
        return test_configuration(setting_SaveFullCore, setting_CreateCoreBacktrace);
    if (argc < 8)
    {
        error_msg_and_die("Usage: %s SIGNO CORE_SIZE_LIMIT PID UID GID TIME BINARY_NAME GLOBAL_PID [TID]", argv[0]);
    }
    if (strchr(argv[1], ' '))
    {
        int i;
        for (i = 1; argv[i]; i++)
        {
            strchrnul(argv[i], ' ')[0] = '\0';
        }
    }
    errno = 0;
    const char* signal_str = argv[1];
    int signal_no = xatoi_positive(signal_str);
    off_t ulimit_c = strtoull(argv[2], NULL, 10);
    if (ulimit_c < 0)  
    {
        ulimit_c = ~((off_t)1 << (sizeof(off_t)*8-1));
    }
    const char *pid_str = argv[3];
    pid_t local_pid = xatoi_positive(argv[3]);
    uid_t uid = xatoi_positive(argv[4]);
    if (errno || local_pid <= 0)
    {
        perror_msg_and_die("PID '%s' or limit '%s' is bogus", argv[3], argv[2]);
    }
    {
        char *s = xmalloc_fopen_fgetline_fclose(VAR_RUN"/abrt/saved_core_pattern");
        if (s && s[0] != '|')
            core_basename = s;
        else
            free(s);
    }
    const char *global_pid_str = argv[8];
    pid_t pid = xatoi_positive(argv[8]);
    pid_t tid = -1;
    const char *tid_str = argv[9];
    if (tid_str)
    {
        tid = xatoi_positive(tid_str);
    }
    char path[PATH_MAX];
    char *executable = get_executable(pid);
    if (executable && strstr(executable, "/abrt-hook-ccpp"))
    {
        error_msg_and_die("PID %lu is '%s', not dumping it to avoid recursion",
                        (long)pid, executable);
    }
    user_pwd = get_cwd(pid);  
    log_notice("user_pwd:'%s'", user_pwd);
    sprintf(path, "/proc/%lu/status", (long)pid);
    char *proc_pid_status = xmalloc_xopen_read_close(path,   NULL);
    uid_t fsuid = uid;
    uid_t tmp_fsuid = get_fsuid(proc_pid_status);
    if (tmp_fsuid < 0)
        perror_msg_and_die("Can't parse 'Uid: line' in /proc/%lu/status", (long)pid);
    const int fsgid = get_fsgid(proc_pid_status);
    if (fsgid < 0)
        error_msg_and_die("Can't parse 'Gid: line' in /proc/%lu/status", (long)pid);
    int suid_policy = dump_suid_policy();
    if (tmp_fsuid != uid)
    {
        fsuid = 0;
        if (suid_policy == DUMP_SUID_UNSAFE)
            fsuid = tmp_fsuid;
        else
        {
            g_user_core_flags = O_EXCL;
            g_need_nonrelative = 1;
        }
    }
    int user_core_fd = -1;
    if (setting_MakeCompatCore && ulimit_c != 0)
        user_core_fd = open_user_core(uid, fsuid, fsgid, pid, &argv[1]);
    if (executable == NULL)
    {
        error_msg("Can't read /proc/%lu/exe link", (long)pid);
        return create_user_core(user_core_fd, pid, ulimit_c);
    }
    const char *signame = NULL;
    if (!signal_is_fatal(signal_no, &signame))
        return create_user_core(user_core_fd, pid, ulimit_c);  
    const int abrtd_running = daemon_is_ok();
    if (!setting_StandaloneHook && !abrtd_running)
    {
        log("abrtd is not running. If it crashed, "
            "/proc/sys/kernel/core_pattern contains a stale value, "
            "consider resetting it to 'core'"
        );
        return create_user_core(user_core_fd, pid, ulimit_c);
    }
    if (setting_StandaloneHook)
        ensure_writable_dir(g_settings_dump_location, DEFAULT_DUMP_LOCATION_MODE, "abrt");
    if (g_settings_nMaxCrashReportsSize > 0)
    {
        if (low_free_space(g_settings_nMaxCrashReportsSize, g_settings_dump_location))
            return create_user_core(user_core_fd, pid, ulimit_c);
    }
    snprintf(path, sizeof(path), "%s/last-ccpp", g_settings_dump_location);
    if (check_recent_crash_file(path, executable))
    {
        return create_user_core(user_core_fd, pid, ulimit_c);
    }
    const char *last_slash = strrchr(executable, '/');
    if (last_slash && strncmp(++last_slash, "abrt", 4) == 0)
    {
        if (g_settings_debug_level == 0)
        {
            log_warning("Ignoring crash of %s (SIG%s).",
                        executable, signame ? signame : signal_str);
            goto cleanup_and_exit;
        }
        if (snprintf(path, sizeof(path), "%s/%s-coredump", g_settings_dump_location, last_slash) >= sizeof(path))
            error_msg_and_die("Error saving '%s': truncated long file path", path);
        int abrt_core_fd = xopen3(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
        off_t core_size = copyfd_eof(STDIN_FILENO, abrt_core_fd, COPYFD_SPARSE);
        if (core_size < 0 || fsync(abrt_core_fd) != 0)
        {
            unlink(path);
            error_msg_and_die("Error saving '%s'", path);
        }
        log_notice("Saved core dump of pid %lu (%s) to %s (%llu bytes)", (long)pid, executable, path, (long long)core_size);
        err = 0;
        goto cleanup_and_exit;
    }
    unsigned path_len = snprintf(path, sizeof(path), "%s/ccpp-%s-%lu.new",
            g_settings_dump_location, iso_date_string(NULL), (long)pid);
    if (path_len >= (sizeof(path) - sizeof("/"FILENAME_COREDUMP)))
    {
        return create_user_core(user_core_fd, pid, ulimit_c);
    }
    dd = dd_create(path,  0, DEFAULT_DUMP_DIR_MODE);
    if (dd)
    {
        char source_filename[sizeof("/proc/%lu/somewhat_long_name") + sizeof(long)*3];
        int source_base_ofs = sprintf(source_filename, "/proc/%lu/root", (long)pid);
        source_base_ofs -= strlen("root");
        char *rootdir = process_has_own_root(pid) ? get_rootdir(pid) : NULL;
        if (g_settings_explorechroots)
        {
            dd_create_basic_files(dd, fsuid, (rootdir != NULL) ? source_filename : NULL);
        }
        else
        {
            dd_create_basic_files(dd, fsuid, NULL);
        }
        char *dest_filename = concat_path_file(dd->dd_dirname, "also_somewhat_longish_name");
        char *dest_base = strrchr(dest_filename, '/') + 1;
        strcpy(source_filename + source_base_ofs, "maps");
        dd_copy_file(dd, FILENAME_MAPS, source_filename);
        strcpy(source_filename + source_base_ofs, "limits");
        dd_copy_file(dd, FILENAME_LIMITS, source_filename);
        strcpy(source_filename + source_base_ofs, "cgroup");
        dd_copy_file(dd, FILENAME_CGROUP, source_filename);
        strcpy(source_filename + source_base_ofs, "mountinfo");
        dd_copy_file(dd, FILENAME_MOUNTINFO, source_filename);
        strcpy(dest_base, FILENAME_OPEN_FDS);
        strcpy(source_filename + source_base_ofs, "fd");
        dump_fd_info_ext(dest_filename, source_filename, dd->dd_uid, dd->dd_gid);
        strcpy(dest_base, FILENAME_NAMESPACES);
        dump_namespace_diff_ext(dest_filename, 1, pid, dd->dd_uid, dd->dd_gid);
        free(dest_filename);
        char *tmp = NULL;
        get_env_variable(pid, "container", &tmp);
        if (tmp != NULL)
        {
            dd_save_text(dd, FILENAME_CONTAINER, tmp);
            free(tmp);
            tmp = NULL;
        }
        get_env_variable(pid, "container_uuid", &tmp);
        if (tmp != NULL)
        {
            dd_save_text(dd, FILENAME_CONTAINER_UUID, tmp);
            free(tmp);
        }
        const int containerized = (rootdir != NULL && strcmp(rootdir, "/") == 0);
        if (containerized)
        {
            log_debug("Process %d is considered to be containerized", pid);
            pid_t container_pid;
            if (get_pid_of_container(pid, &container_pid) == 0)
            {
                char *container_cmdline = get_cmdline(container_pid);
                dd_save_text(dd, FILENAME_CONTAINER_CMDLINE, container_cmdline);
                free(container_cmdline);
            }
        }
        dd_save_text(dd, FILENAME_ANALYZER, "abrt-ccpp");
        dd_save_text(dd, FILENAME_TYPE, "CCpp");
        dd_save_text(dd, FILENAME_EXECUTABLE, executable);
        dd_save_text(dd, FILENAME_PID, pid_str);
        dd_save_text(dd, FILENAME_GLOBAL_PID, global_pid_str);
        dd_save_text(dd, FILENAME_PROC_PID_STATUS, proc_pid_status);
        if (user_pwd)
            dd_save_text(dd, FILENAME_PWD, user_pwd);
        if (tid_str)
            dd_save_text(dd, FILENAME_TID, tid_str);
        if (rootdir)
        {
            if (strcmp(rootdir, "/") != 0)
                dd_save_text(dd, FILENAME_ROOTDIR, rootdir);
        }
        free(rootdir);
        char *reason = xasprintf("%s killed by SIG%s",
                                 last_slash, signame ? signame : signal_str);
        dd_save_text(dd, FILENAME_REASON, reason);
        free(reason);
        char *cmdline = get_cmdline(pid);
        dd_save_text(dd, FILENAME_CMDLINE, cmdline ? : "");
        free(cmdline);
        char *environ = get_environ(pid);
        dd_save_text(dd, FILENAME_ENVIRON, environ ? : "");
        free(environ);
        char *fips_enabled = xmalloc_fopen_fgetline_fclose("/proc/sys/crypto/fips_enabled");
        if (fips_enabled)
        {
            if (strcmp(fips_enabled, "0") != 0)
                dd_save_text(dd, "fips_enabled", fips_enabled);
            free(fips_enabled);
        }
        dd_save_text(dd, FILENAME_ABRT_VERSION, VERSION);
        long unsigned lck_bytes = ULONG_MAX;
        const char *vmlck = strstr(proc_pid_status, "VmLck:");
        if (vmlck == NULL)
            error_msg("/proc/%s/status does not contain 'VmLck:' line", pid_str);
        else if (1 != sscanf(vmlck + 6, "%lu kB\n", &lck_bytes))
            error_msg("Failed to parse 'VmLck:' line in /proc/%s/status", pid_str);
        if (lck_bytes)
        {
            log_notice("Process %s of user %lu has locked memory",
                        pid_str, (long unsigned)uid);
            dd_mark_as_notreportable(dd, "The process had locked memory "
                    "which usually indicates efforts to protect sensitive "
                    "data (passwords) from being written to disk.\n"
                    "In order to avoid sensitive information leakages, "
                    "ABRT will not allow you to report this problem to "
                    "bug tracking tools");
        }
        if (setting_SaveBinaryImage)
        {
            if (save_crashing_binary(pid, dd))
            {
                error_msg("Error saving '%s'", path);
                goto cleanup_and_exit;
            }
        }
        off_t core_size = 0;
        if (setting_SaveFullCore)
        {
            strcpy(path + path_len, "/"FILENAME_COREDUMP);
            int abrt_core_fd = create_or_die(path, user_core_fd);
            core_size = copyfd_sparse(STDIN_FILENO, abrt_core_fd, user_core_fd, ulimit_c);
            close_user_core(user_core_fd, core_size);
            if (fsync(abrt_core_fd) != 0 || close(abrt_core_fd) != 0 || core_size < 0)
            {
                unlink(path);
                error_msg("Error writing '%s'", path);
                goto cleanup_and_exit;
            }
        }
        else
        {
            create_user_core(user_core_fd, pid, ulimit_c);
        }
        user_core_fd = -1;
#if 0
        {
            char *java_log = xasprintf("/tmp/jvm-%lu/hs_error.log", (long)pid);
            int src_fd = open(java_log, O_RDONLY);
            free(java_log);
            if (src_fd < 0)
            {
                java_log = xasprintf("%s/hs_err_pid%lu.log", user_pwd, (long)pid);
                src_fd = open(java_log, O_RDONLY);
                free(java_log);
            }
            if (src_fd >= 0)
            {
                strcpy(path + path_len, "/hs_err.log");
                int dst_fd = create_or_die(path, user_core_fd);
                off_t sz = copyfd_eof(src_fd, dst_fd, COPYFD_SPARSE);
                if (close(dst_fd) != 0 || sz < 0)
                {
                    error_msg("Error saving '%s'", path);
                    goto cleanup_and_exit;
                }
                close(src_fd);
            }
        }
#endif
        if (tid > 0 && setting_CreateCoreBacktrace)
            create_core_backtrace(tid, executable, signal_no, dd);
        dd_close(dd);
        dd = NULL;
        path[path_len] = '\0';  
        if (abrtd_running && setting_SaveContainerizedPackageData && containerized)
        {    
            sprintf(source_filename, "/proc/%lu/root", (long)pid);
            const char *cmd_args[6];
            cmd_args[0] = BIN_DIR"/abrt-action-save-package-data";
            cmd_args[1] = "-d";
            cmd_args[2] = path;
            cmd_args[3] = "-r";
            cmd_args[4] = source_filename;
            cmd_args[5] = NULL;
            pid_t pid = fork_execv_on_steroids(0, (char **)cmd_args, NULL, NULL, path, 0);
            int stat;
            safe_waitpid(pid, &stat, 0);
        }
        char *newpath = xstrndup(path, path_len - (sizeof(".new")-1));
        if (rename(path, newpath) == 0)
            strcpy(path, newpath);
        free(newpath);
        if (core_size > 0)
            log_notice("Saved core dump of pid %lu (%s) to %s (%llu bytes)",
                       (long)pid, executable, path, (long long)core_size);
        if (abrtd_running)
            notify_new_path(path);
        if (g_settings_nMaxCrashReportsSize > 0)
        {
            unsigned maxsize = g_settings_nMaxCrashReportsSize + g_settings_nMaxCrashReportsSize / 4;
            maxsize |= 63;
            trim_problem_dirs(g_settings_dump_location, maxsize * (double)(1024*1024), path);
        }
        err = 0;
    }
    else
    {
        return create_user_core(user_core_fd, pid, ulimit_c);
    }
cleanup_and_exit:
    if (dd)
        dd_delete(dd);
    if (user_core_fd >= 0)
        unlinkat(dirfd(proc_cwd), core_basename,  0);
    if (proc_cwd != NULL)
        closedir(proc_cwd);
    return err;
}