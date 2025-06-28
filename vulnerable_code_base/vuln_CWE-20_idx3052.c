sctp_disposition_t sctp_sf_do_asconf_ack(struct net *net,
					 const struct sctp_endpoint *ep,
					 const struct sctp_association *asoc,
					 const sctp_subtype_t type, void *arg,
					 sctp_cmd_seq_t *commands)
{
	struct sctp_chunk	*asconf_ack = arg;
	struct sctp_chunk	*last_asconf = asoc->addip_last_asconf;
	struct sctp_chunk	*abort;
	struct sctp_paramhdr	*err_param = NULL;
	sctp_addiphdr_t		*addip_hdr;
	__u32			sent_serial, rcvd_serial;
	if (!sctp_vtag_verify(asconf_ack, asoc)) {
		sctp_add_cmd_sf(commands, SCTP_CMD_REPORT_BAD_TAG,
				SCTP_NULL());
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);
	}
	if (!net->sctp.addip_noauth && !asconf_ack->auth)
		return sctp_sf_discard_chunk(net, ep, asoc, type, arg, commands);
	if (!sctp_chunk_length_valid(asconf_ack, sizeof(sctp_addip_chunk_t)))
		return sctp_sf_violation_chunklen(net, ep, asoc, type, arg,
						  commands);
	addip_hdr = (sctp_addiphdr_t *)asconf_ack->skb->data;
	rcvd_serial = ntohl(addip_hdr->serial);
	if (!sctp_verify_asconf(asoc,
	    (sctp_paramhdr_t *)addip_hdr->params,
	    (void *)asconf_ack->chunk_end,
	    &err_param))
		return sctp_sf_violation_paramlen(net, ep, asoc, type, arg,
			   (void *)err_param, commands);
	if (last_asconf) {
		addip_hdr = (sctp_addiphdr_t *)last_asconf->subh.addip_hdr;
		sent_serial = ntohl(addip_hdr->serial);
	} else {
		sent_serial = asoc->addip_serial - 1;
	}
	if (ADDIP_SERIAL_gte(rcvd_serial, sent_serial + 1) &&
	    !(asoc->addip_last_asconf)) {
		abort = sctp_make_abort(asoc, asconf_ack,
					sizeof(sctp_errhdr_t));
		if (abort) {
			sctp_init_cause(abort, SCTP_ERROR_ASCONF_ACK, 0);
			sctp_add_cmd_sf(commands, SCTP_CMD_REPLY,
					SCTP_CHUNK(abort));
		}
		sctp_add_cmd_sf(commands, SCTP_CMD_TIMER_STOP,
				SCTP_TO(SCTP_EVENT_TIMEOUT_T4_RTO));
		sctp_add_cmd_sf(commands, SCTP_CMD_DISCARD_PACKET, SCTP_NULL());
		sctp_add_cmd_sf(commands, SCTP_CMD_SET_SK_ERR,
				SCTP_ERROR(ECONNABORTED));
		sctp_add_cmd_sf(commands, SCTP_CMD_ASSOC_FAILED,
				SCTP_PERR(SCTP_ERROR_ASCONF_ACK));
		SCTP_INC_STATS(net, SCTP_MIB_ABORTEDS);
		SCTP_DEC_STATS(net, SCTP_MIB_CURRESTAB);
		return SCTP_DISPOSITION_ABORT;
	}
	if ((rcvd_serial == sent_serial) && asoc->addip_last_asconf) {
		sctp_add_cmd_sf(commands, SCTP_CMD_TIMER_STOP,
				SCTP_TO(SCTP_EVENT_TIMEOUT_T4_RTO));
		if (!sctp_process_asconf_ack((struct sctp_association *)asoc,
					     asconf_ack)) {
			sctp_add_cmd_sf(commands, SCTP_CMD_SEND_NEXT_ASCONF,
					SCTP_NULL());
			return SCTP_DISPOSITION_CONSUME;
		}
		abort = sctp_make_abort(asoc, asconf_ack,
					sizeof(sctp_errhdr_t));
		if (abort) {
			sctp_init_cause(abort, SCTP_ERROR_RSRC_LOW, 0);
			sctp_add_cmd_sf(commands, SCTP_CMD_REPLY,
					SCTP_CHUNK(abort));
		}
		sctp_add_cmd_sf(commands, SCTP_CMD_DISCARD_PACKET, SCTP_NULL());
		sctp_add_cmd_sf(commands, SCTP_CMD_SET_SK_ERR,
				SCTP_ERROR(ECONNABORTED));
		sctp_add_cmd_sf(commands, SCTP_CMD_ASSOC_FAILED,
				SCTP_PERR(SCTP_ERROR_ASCONF_ACK));
		SCTP_INC_STATS(net, SCTP_MIB_ABORTEDS);
		SCTP_DEC_STATS(net, SCTP_MIB_CURRESTAB);
		return SCTP_DISPOSITION_ABORT;
	}
	return SCTP_DISPOSITION_DISCARD;
}