void sctp_assoc_update(struct sctp_association *asoc,
		       struct sctp_association *new)
{
	struct sctp_transport *trans;
	struct list_head *pos, *temp;
	asoc->c = new->c;
	asoc->peer.rwnd = new->peer.rwnd;
	asoc->peer.sack_needed = new->peer.sack_needed;
	asoc->peer.auth_capable = new->peer.auth_capable;
	asoc->peer.i = new->peer.i;
	sctp_tsnmap_init(&asoc->peer.tsn_map, SCTP_TSN_MAP_INITIAL,
			 asoc->peer.i.initial_tsn, GFP_ATOMIC);
	list_for_each_safe(pos, temp, &asoc->peer.transport_addr_list) {
		trans = list_entry(pos, struct sctp_transport, transports);
		if (!sctp_assoc_lookup_paddr(new, &trans->ipaddr)) {
			sctp_assoc_rm_peer(asoc, trans);
			continue;
		}
		if (asoc->state >= SCTP_STATE_ESTABLISHED)
			sctp_transport_reset(trans);
	}
	if (asoc->state >= SCTP_STATE_ESTABLISHED) {
		asoc->next_tsn = new->next_tsn;
		asoc->ctsn_ack_point = new->ctsn_ack_point;
		asoc->adv_peer_ack_point = new->adv_peer_ack_point;
		sctp_ssnmap_clear(asoc->ssnmap);
		sctp_ulpq_flush(&asoc->ulpq);
		asoc->overall_error_count = 0;
	} else {
		list_for_each_entry(trans, &new->peer.transport_addr_list,
				transports) {
			if (!sctp_assoc_lookup_paddr(asoc, &trans->ipaddr))
				sctp_assoc_add_peer(asoc, &trans->ipaddr,
						    GFP_ATOMIC, trans->state);
		}
		asoc->ctsn_ack_point = asoc->next_tsn - 1;
		asoc->adv_peer_ack_point = asoc->ctsn_ack_point;
		if (!asoc->ssnmap) {
			asoc->ssnmap = new->ssnmap;
			new->ssnmap = NULL;
		}
		if (!asoc->assoc_id) {
			sctp_assoc_set_id(asoc, GFP_ATOMIC);
		}
	}
	kfree(asoc->peer.peer_random);
	asoc->peer.peer_random = new->peer.peer_random;
	new->peer.peer_random = NULL;
	kfree(asoc->peer.peer_chunks);
	asoc->peer.peer_chunks = new->peer.peer_chunks;
	new->peer.peer_chunks = NULL;
	kfree(asoc->peer.peer_hmacs);
	asoc->peer.peer_hmacs = new->peer.peer_hmacs;
	new->peer.peer_hmacs = NULL;
	sctp_auth_key_put(asoc->asoc_shared_key);
	sctp_auth_asoc_init_active_key(asoc, GFP_ATOMIC);
}