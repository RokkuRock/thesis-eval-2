compat_pkalg_proposal(struct ssh *ssh, char *pkalg_prop)
{
	if (!(ssh->compat & SSH_BUG_RSASIGMD5))
		return pkalg_prop;
	debug2_f("original public key proposal: %s", pkalg_prop);
	if ((pkalg_prop = match_filter_denylist(pkalg_prop, "ssh-rsa")) == NULL)
		fatal("match_filter_denylist failed");
	debug2_f("compat public key proposal: %s", pkalg_prop);
	if (*pkalg_prop == '\0')
		fatal("No supported PK algorithms found");
	return pkalg_prop;
}