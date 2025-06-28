int audio_level_json(struct list *levell,
		     const char *userid_self, const char *clientid_self,
		     char **json_str, char **anon_str)
{
	struct json_object *jobj;
	struct json_object *jarr;
	char uid_anon[ANON_ID_LEN];
	char cid_anon[ANON_CLIENT_LEN];
	struct mbuf *pmb = NULL;
	int err = 0;
	struct le *le;
	if (!levell || !json_str)
		return EINVAL;
	jobj = jzon_alloc_object();
	if (!jobj)
		return ENOMEM;
	jarr = jzon_alloc_array();
	if (!jarr) {
		err = ENOMEM;
		goto out;
	}
	if (anon_str) {
		pmb = mbuf_alloc(512);
		mbuf_printf(pmb, "%zu levels: ", list_count(levell));
	}
	LIST_FOREACH(levell, le) {
		struct audio_level *a = le->data;
		struct json_object *ja;
		const char *userid = a->userid;
		const char *clientid = a->clientid;
		if (a->is_self) {
			if (userid_self)
				userid = userid_self;
			if (clientid_self)
				clientid = clientid_self;
		}
		ja = jzon_alloc_object();
		if (ja) {
			jzon_add_str(ja, "userid", userid);
			jzon_add_str(ja, "clientid", clientid);
			jzon_add_int(ja, "audio_level",
				     (int32_t)a->aulevel_smooth);
			jzon_add_int(ja, "audio_level_now",
				     (int32_t)a->aulevel);
		}
		json_object_array_add(jarr, ja);
		if (pmb) {
			anon_id(uid_anon, userid);
			anon_client(cid_anon, clientid);
			mbuf_printf(pmb, "{[%s.%s] audio_level: %d/%d}",
				    uid_anon, cid_anon,
				    a->aulevel_smooth, a->aulevel);
			if (le != levell->tail)
				mbuf_printf(pmb, ",");
		}		
	}
	json_object_object_add(jobj, "audio_levels", jarr);
	if (pmb) {
		pmb->pos = 0;
		mbuf_strdup(pmb, anon_str, pmb->end);
		mem_deref(pmb);
	}
	jzon_encode(json_str, jobj);
 out:	
	mem_deref(jobj);
	return err;
}