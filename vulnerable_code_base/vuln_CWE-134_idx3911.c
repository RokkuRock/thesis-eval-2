static void prekey_handler(const char *userid,
			   const uint8_t *key, size_t key_len,
			   uint16_t id, const char *clientid,
			   bool last, void *arg)
{
	struct session *sess;
	char lclientid[64];
	int err;
	output("prekey_handler: %zu bytes, user:%s[%u] -> %s\n",
	       key_len, userid, id, clientid);
	err = client_id_load(lclientid, sizeof(lclientid));
	if (err) {
		debug("my clientid not set -- cannot store prekeys\n");
		return;
	}
	sess = cryptobox_session_find(g_cryptobox, userid, clientid, lclientid);
	if (sess) {
		output("prekey: session found\n");
	}
	else {
		info("conv: adding key to cryptobox for clientid=%s\n",
		     clientid);
		err = cryptobox_session_add_send(g_cryptobox, userid, clientid, lclientid,
						 key, key_len);
		if (err) {
			warning("cryptobox_session_add_send failed (%m)\n",
				err);
		}
	}
}