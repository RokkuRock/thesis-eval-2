compat_kex_proposal(struct ssh *ssh, char *p)
{
	if ((ssh->compat & (SSH_BUG_CURVE25519PAD|SSH_OLD_DHGEX)) == 0)
		return p;
	debug2_f("original KEX proposal: %s", p);
	if ((ssh->compat & SSH_BUG_CURVE25519PAD) != 0)
		if ((p = match_filter_denylist(p,
		    "curve25519-sha256@libssh.org")) == NULL)
			fatal("match_filter_denylist failed");
	if ((ssh->compat & SSH_OLD_DHGEX) != 0) {
		if ((p = match_filter_denylist(p,
		    "diffie-hellman-group-exchange-sha256,"
		    "diffie-hellman-group-exchange-sha1")) == NULL)
			fatal("match_filter_denylist failed");
	}
	debug2_f("compat KEX proposal: %s", p);
	if (*p == '\0')
		fatal("No supported key exchange algorithms found");
	return p;
}