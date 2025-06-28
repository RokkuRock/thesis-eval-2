const char *jsi_GetHomeDir(Jsi_Interp *interp) {
    const char *str = NULL;
    if (interp->homeDir)
        return interp->homeDir;
#ifdef __WIN32
    str = getenv("USERPROFILE");  
#else
    if ((str = getenv("HOME")) == NULL) {
        struct passwd pwd, *pw;
        char buf[20000];
        if (getpwuid_r(getuid(), &pwd, buf, sizeof(buf), &pw) == 0 && pw->pw_dir)        
            str = pw->pw_dir;
    }
#endif
    if (!str) {
        Jsi_LogBug("no home dir");
        str = "/";
    }
#ifdef JSI_LITE_ONLY
    return str;
#else
    return (interp->homeDir = Jsi_KeyAdd(interp, str));
#endif
}