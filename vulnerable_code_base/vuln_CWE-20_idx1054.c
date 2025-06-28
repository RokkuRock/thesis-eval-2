sctp_disposition_t sctp_sf_do_asconf(struct net *net,
				     const struct sctp_endpoint *ep,
				     const struct sctp_association *asoc,
				     const sctp_subtype_t type, void *arg,
				     sctp_cmd_seq_t *commands)
{
	struct sctp_chunk	*chunk = arg;
	struct sctp_chunk	*asconf_ack = NULL;
	struct sctp_paramhdr	*err_param = NULL;
	sctp_addiphdr_t		*hdr;
	union sctp_addr_param	*addr_param;
	__u32			serial;
	int			length;
	if (!sctp_vtag_verify(chunk, asoc)) {
		sctp_add_cmd_sf(commands, SCTP_CMD_REPORT_BAD_TAG,
				SCTP_NULL());
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);
	}
	if (!net->sctp.addip_noauth && !chunk->auth)
		return sctp_sf_discard_chunk(net, ep, asoc, type, arg, commands);
	if (!sctp_chunk_length_valid(chunk, sizeof(sctp_addip_chunk_t)))
		return sctp_sf_violation_chunklen(net, ep, asoc, type, arg,
						  commands);
	hdr = (sctp_addiphdr_t *)chunk->skb->data;
	serial = ntohl(hdr->serial);
	addr_param = (union sctp_addr_param *)hdr->params;
	length = ntohs(addr_param->p.length);
	if (length < sizeof(sctp_paramhdr_t))
		return sctp_sf_violation_paramlen(net, ep, asoc, type, arg,
			   (void *)addr_param, commands);
	if (!sctp_verify_asconf(asoc,
			    (sctp_paramhdr_t *)((void *)addr_param + length),
			    (void *)chunk->chunk_end,
			    &err_param))
		return sctp_sf_violation_paramlen(net, ep, asoc, type, arg,
						  (void *)err_param, commands);
	if (serial == asoc->peer.addip_serial + 1) {
		if (!chunk->has_asconf)
			sctp_assoc_clean_asconf_ack_cache(asoc);
		asconf_ack = sctp_process_asconf((struct sctp_association *)
						 asoc, chunk);
		if (!asconf_ack)
			return SCTP_DISPOSITION_NOMEM;
	} else if (serial < asoc->peer.addip_serial + 1) {
		asconf_ack = sctp_assoc_lookup_asconf_ack(asoc, hdr->serial);
		if (!asconf_ack)
			return SCTP_DISPOSITION_DISCARD;
		asconf_ack->transport = NULL;
	} else {
		return SCTP_DISPOSITION_DISCARD;
	}
	asconf_ack->dest = chunk->source;
	sctp_add_cmd_sf(commands, SCTP_CMD_REPLY, SCTP_CHUNK(asconf_ack));
	if (asoc->new_transport) {
		sctp_sf_heartbeat(ep, asoc, type, asoc->new_transport, commands);
		((struct sctp_association *)asoc)->new_transport = NULL;
	}
	return SCTP_DISPOSITION_CONSUME;
}