static int unix_getpw(UNUSED void *instance, REQUEST *request,
		      VALUE_PAIR **vp_list)
{
	const char	*name;
	const char	*encrypted_pass;
#ifdef HAVE_GETSPNAM
	struct spwd	*spwd = NULL;
#endif
#ifdef OSFC2
	struct pr_passwd *pr_pw;
#else
	struct passwd	*pwd;
#endif
#ifdef HAVE_GETUSERSHELL
	char		*shell;
#endif
	VALUE_PAIR	*vp;
	if (!request->username) {
		return RLM_MODULE_NOOP;
	}
	name = (char *)request->username->vp_strvalue;
	encrypted_pass = NULL;
#ifdef OSFC2
	if ((pr_pw = getprpwnam(name)) == NULL)
		return RLM_MODULE_NOTFOUND;
	encrypted_pass = pr_pw->ufld.fd_encrypt;
	if (pr_pw->uflg.fg_lock!=1) {
		radlog(L_AUTH, "rlm_unix: [%s]: account locked", name);
		return RLM_MODULE_USERLOCK;
	}
#else  
	if ((pwd = getpwnam(name)) == NULL) {
		return RLM_MODULE_NOTFOUND;
	}
	encrypted_pass = pwd->pw_passwd;
#endif  
#ifdef HAVE_GETSPNAM
	if ((encrypted_pass == NULL) || (strlen(encrypted_pass) < 10)) {
		if ((spwd = getspnam(name)) == NULL) {
			return RLM_MODULE_NOTFOUND;
		}
		encrypted_pass = spwd->sp_pwdp;
	}
#endif	 
#ifndef OSFC2
#ifdef DENY_SHELL
	if (strcmp(pwd->pw_shell, DENY_SHELL) == 0) {
		radlog_request(L_AUTH, 0, request,
			       "rlm_unix: [%s]: invalid shell", name);
		return RLM_MODULE_REJECT;
	}
#endif
#ifdef HAVE_GETUSERSHELL
	while ((shell = getusershell()) != NULL) {
		if (strcmp(shell, pwd->pw_shell) == 0 ||
		    strcmp(shell, "/RADIUSD/ANY/SHELL") == 0) {
			break;
		}
	}
	endusershell();
	if (shell == NULL) {
		radlog_request(L_AUTH, 0, request, "[%s]: invalid shell [%s]",
		       name, pwd->pw_shell);
		return RLM_MODULE_REJECT;
	}
#endif
#endif  
#if defined(HAVE_GETSPNAM) && !defined(M_UNIX)
	if (spwd && spwd->sp_expire > 0 &&
	    (request->timestamp / 86400) > spwd->sp_expire) {
		radlog_request(L_AUTH, 0, request, "[%s]: password has expired", name);
		return RLM_MODULE_REJECT;
	}
#endif
#if defined(__FreeBSD__) || defined(bsdi) || defined(_PWF_EXPIRE)
	if ((pwd->pw_expire > 0) &&
	    (request->timestamp > pwd->pw_expire)) {
		radlog_request(L_AUTH, 0, request, "[%s]: password has expired", name);
		return RLM_MODULE_REJECT;
	}
#endif
	if (encrypted_pass[0] == 0)
		return RLM_MODULE_NOOP;
	vp = pairmake("Crypt-Password", encrypted_pass, T_OP_SET);
	if (!vp) return RLM_MODULE_FAIL;
	pairmove(vp_list, &vp);
	pairfree(&vp);		 
	return RLM_MODULE_UPDATED;
}