int econn_message_encode(char **strp, const struct econn_message *msg)
{
	struct json_object *jobj = NULL;
	char *str = NULL;
	int err;
	if (!strp || !msg)
		return EINVAL;
	err = jzon_creatf(&jobj, "sss",
			  "version", econn_proto_version,
			  "type",   econn_msg_name(msg->msg_type),
			  "sessid", msg->sessid_sender);
	if (err)
		return err;
	if (str_isset(msg->src_userid)) {
		err = jzon_add_str(jobj, "src_userid", "%s", msg->src_userid);
		if (err)
			goto out;
	}
	if (str_isset(msg->src_clientid)) {
		err = jzon_add_str(jobj, "src_clientid",
				   "%s", msg->src_clientid);
		if (err)
			goto out;
	}
	if (str_isset(msg->dest_userid)) {
		err = jzon_add_str(jobj, "dest_userid",
				   "%s", msg->dest_userid);
		if (err)
			goto out;
	}
	if (str_isset(msg->dest_clientid)) {
		err = jzon_add_str(jobj, "dest_clientid",
				   "%s", msg->dest_clientid);
		if (err)
			goto out;
	}
	err = jzon_add_bool(jobj, "resp", msg->resp);
	if (err)
		goto out;
	switch (msg->msg_type) {
	case ECONN_SETUP:
	case ECONN_GROUP_SETUP:
	case ECONN_UPDATE:
		err = jzon_add_str(jobj, "sdp", "%s", msg->u.setup.sdp_msg);
		if (err)
			goto out;
		if (msg->u.setup.props) {
			err = econn_props_encode(jobj, msg->u.setup.props);
			if (err)
				goto out;
		}
		if (msg->u.setup.url) {
			err = jzon_add_str(jobj, "url", "%s", msg->u.setup.url);
			if (err)
				goto out;
		}
		break;
	case ECONN_CANCEL:
		break;
	case ECONN_HANGUP:
		break;
	case ECONN_REJECT:
		break;
	case ECONN_PROPSYNC:
		if (!msg->u.propsync.props) {
			warning("propsync: missing props\n");
			err = EINVAL;
			goto out;
		}
		err = econn_props_encode(jobj, msg->u.propsync.props);
		if (err)
			goto out;
		break;
	case ECONN_GROUP_START:
		if (msg->u.groupstart.props) {
			err = econn_props_encode(jobj, msg->u.groupstart.props);
			if (err)
				goto out;
		}
		break;
	case ECONN_GROUP_LEAVE:
	case ECONN_GROUP_CHECK:
		break;
	case ECONN_CONF_CONN:
		if (msg->u.confconn.turnc > 0) {
			err = zapi_iceservers_encode(jobj,
						     msg->u.confconn.turnv,
						     msg->u.confconn.turnc);
			if (err)
				goto out;
		}
		jzon_add_bool(jobj, "update",
			      msg->u.confconn.update);
		jzon_add_str(jobj, "tool",
			      msg->u.confconn.tool);
		jzon_add_str(jobj, "toolver",
			      msg->u.confconn.toolver);
		jzon_add_int(jobj, "status",
			      msg->u.confconn.status);
		jzon_add_bool(jobj, "selective_audio",
			      msg->u.confconn.selective_audio);
		jzon_add_bool(jobj, "selective_video",
			      msg->u.confconn.selective_video);
		jzon_add_int(jobj, "vstreams",
			      msg->u.confconn.vstreams);
		break;
	case ECONN_CONF_START:
		jzon_add_str(jobj, "sft_url", "%s", msg->u.confstart.sft_url);
		jzon_add_base64(jobj, "secret",
				msg->u.confstart.secret, msg->u.confstart.secretlen);
		jzon_add_str(jobj, "timestamp", "%llu", msg->u.confstart.timestamp);
		jzon_add_str(jobj, "seqno", "%u", msg->u.confstart.seqno);
		if (msg->u.confstart.props) {
			err = econn_props_encode(jobj, msg->u.confstart.props);
			if (err)
				goto out;
		}
		break;
	case ECONN_CONF_CHECK:
		jzon_add_str(jobj, "sft_url", "%s", msg->u.confcheck.sft_url);
		jzon_add_base64(jobj, "secret",
				msg->u.confcheck.secret, msg->u.confcheck.secretlen);
		jzon_add_str(jobj, "timestamp", "%llu", msg->u.confcheck.timestamp);
		jzon_add_str(jobj, "seqno", "%u", msg->u.confcheck.seqno);
		break;
	case ECONN_CONF_END:
		break;
	case ECONN_CONF_PART:
		jzon_add_bool(jobj, "should_start",
			      msg->u.confpart.should_start);
		jzon_add_str(jobj, "timestamp", "%llu", msg->u.confpart.timestamp);
		jzon_add_str(jobj, "seqno", "%u", msg->u.confpart.seqno);
		jzon_add_base64(jobj, "entropy",
				msg->u.confpart.entropy, msg->u.confpart.entropylen);
		econn_parts_encode(jobj, &msg->u.confpart.partl);
		break;
	case ECONN_CONF_KEY:
		econn_keys_encode(jobj, &msg->u.confkey.keyl);
		break;
	case ECONN_DEVPAIR_PUBLISH:
		err = zapi_iceservers_encode(jobj,
					     msg->u.devpair_publish.turnv,
					     msg->u.devpair_publish.turnc);
		if (err)
			goto out;
		err = jzon_add_str(jobj, "sdp",
				   "%s", msg->u.devpair_publish.sdp);
		err |= jzon_add_str(jobj, "username",
				    "%s", msg->u.devpair_publish.username);
		if (err)
			goto out;
		break;
	case ECONN_DEVPAIR_ACCEPT:
		err = jzon_add_str(jobj, "sdp",
				   "%s", msg->u.devpair_accept.sdp);
		if (err)
			goto out;
		break;
	case ECONN_ALERT:
		err  = jzon_add_int(jobj, "level", msg->u.alert.level);
		err |= jzon_add_str(jobj, "descr", "%s", msg->u.alert.descr);
		if (err)
			goto out;
		break;
	case ECONN_PING:
		break;
	default:
		warning("econn: dont know how to encode %d\n", msg->msg_type);
		err = EBADMSG;
		break;
	}
	if (err)
		goto out;
	err = jzon_encode(&str, jobj);
	if (err)
		goto out;
 out:
	mem_deref(jobj);
	if (err)
		mem_deref(str);
	else
		*strp = str;
	return err;
}