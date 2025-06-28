static void setup_namespaces(struct lo_data *lo, struct fuse_session *se)
{
    pid_t child;
    char template[] = "virtiofsd-XXXXXX";
    char *tmpdir;
    if (unshare(CLONE_NEWPID | CLONE_NEWNS | CLONE_NEWNET) != 0) {
        fuse_log(FUSE_LOG_ERR, "unshare(CLONE_NEWPID | CLONE_NEWNS): %m\n");
        exit(1);
    }
    child = fork();
    if (child < 0) {
        fuse_log(FUSE_LOG_ERR, "fork() failed: %m\n");
        exit(1);
    }
    if (child > 0) {
        pid_t waited;
        int wstatus;
        setup_wait_parent_capabilities();
        do {
            waited = waitpid(child, &wstatus, 0);
        } while (waited < 0 && errno == EINTR && !se->exited);
        if (se->exited) {
            exit(0);
        }
        if (WIFEXITED(wstatus)) {
            exit(WEXITSTATUS(wstatus));
        }
        exit(1);
    }
    prctl(PR_SET_PDEATHSIG, SIGTERM);
    if (mount(NULL, "/", NULL, MS_REC | MS_SLAVE, NULL) < 0) {
        fuse_log(FUSE_LOG_ERR, "mount(/, MS_REC|MS_SLAVE): %m\n");
        exit(1);
    }
    if (mount("proc", "/proc", "proc",
              MS_NODEV | MS_NOEXEC | MS_NOSUID | MS_RELATIME, NULL) < 0) {
        fuse_log(FUSE_LOG_ERR, "mount(/proc): %m\n");
        exit(1);
    }
    tmpdir = mkdtemp(template);
    if (!tmpdir) {
        fuse_log(FUSE_LOG_ERR, "tmpdir(%s): %m\n", template);
        exit(1);
    }
    if (mount("/proc/self/fd", tmpdir, NULL, MS_BIND, NULL) < 0) {
        fuse_log(FUSE_LOG_ERR, "mount(/proc/self/fd, %s, MS_BIND): %m\n",
                 tmpdir);
        exit(1);
    }
    lo->proc_self_fd = open(tmpdir, O_PATH);
    if (lo->proc_self_fd == -1) {
        fuse_log(FUSE_LOG_ERR, "open(%s, O_PATH): %m\n", tmpdir);
        exit(1);
    }
    if (umount2(tmpdir, MNT_DETACH) < 0) {
        fuse_log(FUSE_LOG_ERR, "umount2(%s, MNT_DETACH): %m\n", tmpdir);
        exit(1);
    }
    if (rmdir(tmpdir) < 0) {
        fuse_log(FUSE_LOG_ERR, "rmdir(%s): %m\n", tmpdir);
    }
}