sudo_passwd_verify(struct passwd *pw, char *pass, sudo_auth *auth, struct sudo_conv_callback *callback)
{
    char sav, *epass;
    char *pw_epasswd = auth->data;
    size_t pw_len;
    int matched = 0;
    debug_decl(sudo_passwd_verify, SUDOERS_DEBUG_AUTH);
    if (pass[0] == '\0')
	debug_return_int(pw_epasswd[0] ? AUTH_FAILURE : AUTH_SUCCESS);
    sav = pass[8];
    pw_len = strlen(pw_epasswd);
    if (pw_len == DESLEN || HAS_AGEINFO(pw_epasswd, pw_len))
	pass[8] = '\0';
    epass = (char *) crypt(pass, pw_epasswd);
    pass[8] = sav;
    if (epass != NULL) {
	if (HAS_AGEINFO(pw_epasswd, pw_len) && strlen(epass) == DESLEN)
	    matched = !strncmp(pw_epasswd, epass, DESLEN);
	else
	    matched = !strcmp(pw_epasswd, epass);
    }
    debug_return_int(matched ? AUTH_SUCCESS : AUTH_FAILURE);
}