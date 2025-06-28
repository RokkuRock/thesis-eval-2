int main(int argc, char **argv)
{
    setlocale(LC_ALL, "");
#if ENABLE_NLS
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);
#endif
    abrt_init(argv);
    const char *program_usage_string = _(
        "& [-y] [-i BUILD_IDS_FILE|-i -] [-e PATH[:PATH]...]\n"
        "\t[-r REPO]\n"
        "\n"
        "Installs debuginfo packages for all build-ids listed in BUILD_IDS_FILE to\n"
        "ABRT system cache."
    );
    enum {
        OPT_v = 1 << 0,
        OPT_y = 1 << 1,
        OPT_i = 1 << 2,
        OPT_e = 1 << 3,
        OPT_r = 1 << 4,
        OPT_s = 1 << 5,
    };
    const char *build_ids = "build_ids";
    const char *exact = NULL;
    const char *repo = NULL;
    const char *size_mb = NULL;
    struct options program_options[] = {
        OPT__VERBOSE(&g_verbose),
        OPT_BOOL  ('y', "yes",         NULL,                   _("Noninteractive, assume 'Yes' to all questions")),
        OPT_STRING('i', "ids",   &build_ids, "BUILD_IDS_FILE", _("- means STDIN, default: build_ids")),
        OPT_STRING('e', "exact",     &exact, "EXACT",          _("Download only specified files")),
        OPT_STRING('r', "repo",       &repo, "REPO",           _("Pattern to use when searching for repos, default: *debug*")),
        OPT_STRING('s', "size_mb", &size_mb, "SIZE_MB",        _("Ignored option")),
        OPT_END()
    };
    const unsigned opts = parse_opts(argc, argv, program_options, program_usage_string);
    const gid_t egid = getegid();
    const gid_t rgid = getgid();
    const uid_t euid = geteuid();
    const gid_t ruid = getuid();
    char *build_ids_self_fd = NULL;
    if (strcmp("-", build_ids) != 0)
    {
        if (setregid(egid, rgid) < 0)
            perror_msg_and_die("setregid(egid, rgid)");
        if (setreuid(euid, ruid) < 0)
            perror_msg_and_die("setreuid(euid, ruid)");
        const int build_ids_fd = open(build_ids, O_RDONLY);
        if (setregid(rgid, egid) < 0)
            perror_msg_and_die("setregid(rgid, egid)");
        if (setreuid(ruid, euid) < 0 )
            perror_msg_and_die("setreuid(ruid, euid)");
        if (build_ids_fd < 0)
            perror_msg_and_die("Failed to open file '%s'", build_ids);
        build_ids_self_fd = xasprintf("/proc/self/fd/%d", build_ids_fd);
    }
    const char *args[11];
    {
        const char *verbs[] = { "", "-v", "-vv", "-vvv" };
        unsigned i = 0;
        args[i++] = EXECUTABLE;
        args[i++] = "--ids";
        args[i++] = (build_ids_self_fd != NULL) ? build_ids_self_fd : "-";
        if (g_verbose > 0)
            args[i++] = verbs[g_verbose <= 3 ? g_verbose : 3];
        if ((opts & OPT_y))
            args[i++] = "-y";
        if ((opts & OPT_e))
        {
            args[i++] = "--exact";
            args[i++] = exact;
        }
        if ((opts & OPT_r))
        {
            args[i++] = "--repo";
            args[i++] = repo;
        }
        args[i++] = "--";
        args[i] = NULL;
    }
    if (egid != rgid)
        IGNORE_RESULT(setregid(egid, egid));
    if (euid != ruid)
    {
        IGNORE_RESULT(setreuid(euid, euid));
#if 1
        static const char *whitelist[] = {
            "REPORT_CLIENT_SLAVE",  
            "LANG",
        };
        const size_t wlsize = sizeof(whitelist)/sizeof(char*);
        char *setlist[sizeof(whitelist)/sizeof(char*)] = { 0 };
        char *p = NULL;
        for (size_t i = 0; i < wlsize; i++)
            if ((p = getenv(whitelist[i])) != NULL)
                setlist[i] = xstrdup(p);
        clearenv();
        for (size_t i = 0; i < wlsize; i++)
            if (setlist[i] != NULL)
            {
                xsetenv(whitelist[i], setlist[i]);
                free(setlist[i]);
            }
#else
        static const char forbid[] =
            "LD_LIBRARY_PATH" "\0"
            "LD_PRELOAD" "\0"
            "LD_TRACE_LOADED_OBJECTS" "\0"
            "LD_BIND_NOW" "\0"
            "LD_AOUT_LIBRARY_PATH" "\0"
            "LD_AOUT_PRELOAD" "\0"
            "LD_NOWARN" "\0"
            "LD_KEEPDIR" "\0"
        ;
        const char *p = forbid;
        do {
            unsetenv(p);
            p += strlen(p) + 1;
        } while (*p);
#endif
        char path_env[] = "PATH=/usr/sbin:/sbin:/usr/bin:/bin:"BIN_DIR":"SBIN_DIR;
        if (euid != 0)
            strcpy(path_env, "PATH=/usr/bin:/bin:"BIN_DIR);
        putenv(path_env);
        umask(0022);
    }
    execvp(EXECUTABLE, (char **)args);
    error_msg_and_die("Can't execute %s", EXECUTABLE);
}