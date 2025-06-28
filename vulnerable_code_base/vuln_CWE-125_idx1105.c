sctp_disposition_t sctp_sf_ootb(struct net *net,
				const struct sctp_endpoint *ep,
				const struct sctp_association *asoc,
				const sctp_subtype_t type,
				void *arg,
				sctp_cmd_seq_t *commands)
{
	struct sctp_chunk *chunk = arg;
	struct sk_buff *skb = chunk->skb;
	sctp_chunkhdr_t *ch;
	sctp_errhdr_t *err;
	__u8 *ch_end;
	int ootb_shut_ack = 0;
	int ootb_cookie_ack = 0;
	SCTP_INC_STATS(net, SCTP_MIB_OUTOFBLUES);
	ch = (sctp_chunkhdr_t *) chunk->chunk_hdr;
	do {
		if (ntohs(ch->length) < sizeof(sctp_chunkhdr_t))
			return sctp_sf_violation_chunklen(net, ep, asoc, type, arg,
						  commands);
		if (SCTP_CID_SHUTDOWN_ACK == ch->type)
			ootb_shut_ack = 1;
		if (SCTP_CID_ABORT == ch->type)
			return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);
		if (SCTP_CID_COOKIE_ACK == ch->type)
			ootb_cookie_ack = 1;
		if (SCTP_CID_ERROR == ch->type) {
			sctp_walk_errors(err, ch) {
				if (SCTP_ERROR_STALE_COOKIE == err->cause) {
					ootb_cookie_ack = 1;
					break;
				}
			}
		}
		ch_end = ((__u8 *)ch) + SCTP_PAD4(ntohs(ch->length));
		if (ch_end > skb_tail_pointer(skb))
			return sctp_sf_violation_chunklen(net, ep, asoc, type, arg,
						  commands);
		ch = (sctp_chunkhdr_t *) ch_end;
	} while (ch_end < skb_tail_pointer(skb));
	if (ootb_shut_ack)
		return sctp_sf_shut_8_4_5(net, ep, asoc, type, arg, commands);
	else if (ootb_cookie_ack)
		return sctp_sf_pdiscard(net, ep, asoc, type, arg, commands);
	else
		return sctp_sf_tabort_8_4_8(net, ep, asoc, type, arg, commands);
}