sctp_disposition_t sctp_sf_do_5_2_4_dupcook(struct net *net,
					const struct sctp_endpoint *ep,
					const struct sctp_association *asoc,
					const sctp_subtype_t type,
					void *arg,
					sctp_cmd_seq_t *commands)
{
	sctp_disposition_t retval;
	struct sctp_chunk *chunk = arg;
	struct sctp_association *new_asoc;
	int error = 0;
	char action;
	struct sctp_chunk *err_chk_p;
	if (!sctp_chunk_length_valid(chunk, sizeof(sctp_chunkhdr_t)))
		return sctp_sf_violation_chunklen(net, ep, asoc, type, arg,
						  commands);
	chunk->subh.cookie_hdr = (struct sctp_signed_cookie *)chunk->skb->data;
	if (!pskb_pull(chunk->skb, ntohs(chunk->chunk_hdr->length) -
					sizeof(sctp_chunkhdr_t)))
		goto nomem;
	new_asoc = sctp_unpack_cookie(ep, asoc, chunk, GFP_ATOMIC, &error,
				      &err_chk_p);
	if (!new_asoc) {
		switch (error) {
		case -SCTP_IERROR_NOMEM:
			goto nomem;
		case -SCTP_IERROR_STALE_COOKIE:
			sctp_send_stale_cookie_err(net, ep, asoc, chunk, commands,
						   err_chk_p);
			return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);
		case -SCTP_IERROR_BAD_SIG:
		default:
			return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);
		}
	}
	action = sctp_tietags_compare(new_asoc, asoc);
	switch (action) {
	case 'A':  
		retval = sctp_sf_do_dupcook_a(net, ep, asoc, chunk, commands,
					      new_asoc);
		break;
	case 'B':  
		retval = sctp_sf_do_dupcook_b(net, ep, asoc, chunk, commands,
					      new_asoc);
		break;
	case 'C':  
		retval = sctp_sf_do_dupcook_c(net, ep, asoc, chunk, commands,
					      new_asoc);
		break;
	case 'D':  
		retval = sctp_sf_do_dupcook_d(net, ep, asoc, chunk, commands,
					      new_asoc);
		break;
	default:  
		retval = sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);
		break;
	}
	sctp_add_cmd_sf(commands, SCTP_CMD_NEW_ASOC, SCTP_ASOC(new_asoc));
	sctp_add_cmd_sf(commands, SCTP_CMD_DELETE_TCB, SCTP_NULL());
	sctp_add_cmd_sf(commands, SCTP_CMD_SET_ASOC,
			 SCTP_ASOC((struct sctp_association *)asoc));
	return retval;
nomem:
	return SCTP_DISPOSITION_NOMEM;
}