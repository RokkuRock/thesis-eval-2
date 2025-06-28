compat_cipher_proposal(struct ssh *ssh, char *cipher_prop)
{
	if (!(ssh->compat & SSH_BUG_BIGENDIANAES))
		return cipher_prop;
	debug2_f("original cipher proposal: %s", cipher_prop);
	if ((cipher_prop = match_filter_denylist(cipher_prop, "aes*")) == NULL)
		fatal("match_filter_denylist failed");
	debug2_f("compat cipher proposal: %s", cipher_prop);
	if (*cipher_prop == '\0')
		fatal("No supported ciphers found");
	return cipher_prop;
}