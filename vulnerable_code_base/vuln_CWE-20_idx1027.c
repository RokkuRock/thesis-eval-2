stf_status ikev2parent_inI1outR1(struct msg_digest *md)
{
	struct state *st = md->st;
	lset_t policy = POLICY_IKEV2_ALLOW;
	struct connection *c = find_host_connection(&md->iface->ip_addr,
						    md->iface->port,
						    &md->sender,
						    md->sender_port,
						    POLICY_IKEV2_ALLOW);
#if 0
	if (c == NULL) {
		pb_stream pre_sa_pbs = sa_pd->pbs;
		policy = preparse_isakmp_sa_body(&pre_sa_pbs);
		c = find_host_connection(&md->iface->ip_addr, pluto_port,
					 (ip_address*)NULL, md->sender_port,
					 policy);
	}
#endif
	if (c == NULL) {
		{
			struct connection *d;
			d = find_host_connection(&md->iface->ip_addr,
						 pluto_port,
						 (ip_address*)NULL,
						 md->sender_port, policy);
			for (; d != NULL; d = d->hp_next) {
				if (d->kind == CK_GROUP) {
				} else {
					if (d->kind == CK_TEMPLATE &&
					    !(d->policy & POLICY_OPPO)) {
						c = d;
						break;
					}
					if (addrinsubnet(&md->sender,
							 &d->spd.that.client)
					    &&
					    (c == NULL ||
					     !subnetinsubnet(&c->spd.that.
							     client,
							     &d->spd.that.
							     client)))
						c = d;
				}
			}
		}
		if (c == NULL) {
			loglog(RC_LOG_SERIOUS, "initial parent SA message received on %s:%u"
			       " but no connection has been authorized%s%s",
			       ip_str(
				       &md->iface->ip_addr),
			       ntohs(portof(&md->iface->ip_addr)),
			       (policy != LEMPTY) ? " with policy=" : "",
			       (policy !=
				LEMPTY) ? bitnamesof(sa_policy_bit_names,
						     policy) : "");
			return STF_FAIL + v2N_NO_PROPOSAL_CHOSEN;
		}
		if (c->kind != CK_TEMPLATE) {
			loglog(RC_LOG_SERIOUS, "initial parent SA message received on %s:%u"
			       " but \"%s\" forbids connection",
			       ip_str(
				       &md->iface->ip_addr), pluto_port,
			       c->name);
			return STF_FAIL + v2N_NO_PROPOSAL_CHOSEN;
		}
		c = rw_instantiate(c, &md->sender, NULL, NULL);
	} else {
		if ((c->kind == CK_TEMPLATE) && c->spd.that.virt) {
			DBG(DBG_CONTROL,
			    DBG_log(
				    "local endpoint has virt (vnet/vhost) set without wildcards - needs instantiation"));
			c = rw_instantiate(c, &md->sender, NULL, NULL);
		} else if ((c->kind == CK_TEMPLATE) &&
			   (c->policy & POLICY_IKEV2_ALLOW_NARROWING)) {
			DBG(DBG_CONTROL,
			    DBG_log(
				    "local endpoint has narrowing=yes - needs instantiation"));
			c = rw_instantiate(c, &md->sender, NULL, NULL);
		}
	}
	DBG_log("found connection: %s\n", c ? c->name : "<none>");
	if (!st) {
		st = new_state();
		memcpy(st->st_icookie, md->hdr.isa_icookie, COOKIE_SIZE);
		get_cookie(FALSE, st->st_rcookie, COOKIE_SIZE, &md->sender);
		initialize_new_state(st, c, policy, 0, NULL_FD,
				     pcim_stranger_crypto);
		st->st_ikev2 = TRUE;
		change_state(st, STATE_PARENT_R1);
		st->st_msgid_lastack = INVALID_MSGID;
		st->st_msgid_nextuse = 0;
		md->st = st;
		md->from_state = STATE_IKEv2_BASE;
	}
	if (force_busy == TRUE) {
		u_char dcookie[SHA1_DIGEST_SIZE];
		chunk_t dc;
		ikev2_get_dcookie( dcookie, st->st_ni, &md->sender,
				   st->st_icookie);
		dc.ptr = dcookie;
		dc.len = SHA1_DIGEST_SIZE;
		if ( md->chain[ISAKMP_NEXT_v2KE] &&
		     md->chain[ISAKMP_NEXT_v2N] &&
		     (md->chain[ISAKMP_NEXT_v2N]->payload.v2n.isan_type ==
		      v2N_COOKIE)) {
			u_int8_t spisize;
			const pb_stream *dc_pbs;
			chunk_t blob;
			DBG(DBG_CONTROLMORE,
			    DBG_log("received a DOS cookie in I1 verify it"));
			spisize =
				md->chain[ISAKMP_NEXT_v2N]->payload.v2n.
				isan_spisize;
			dc_pbs = &md->chain[ISAKMP_NEXT_v2N]->pbs;
			blob.ptr = dc_pbs->cur + spisize;
			blob.len = pbs_left(dc_pbs) - spisize;
			DBG(DBG_CONTROLMORE,
			    DBG_dump_chunk("dcookie received in I1 Packet",
					   blob);
			    DBG_dump("dcookie computed", dcookie,
				     SHA1_DIGEST_SIZE));
			if (memcmp(blob.ptr, dcookie, SHA1_DIGEST_SIZE) != 0) {
				libreswan_log(
					"mismatch in DOS v2N_COOKIE,send a new one");
				SEND_NOTIFICATION_AA(v2N_COOKIE, &dc);
				return STF_FAIL + v2N_INVALID_IKE_SPI;
			}
			DBG(DBG_CONTROLMORE,
			    DBG_log("dcookie received match with computed one"));
		} else {
			DBG(DBG_CONTROLMORE,
			    DBG_log(
				    "busy mode on. receieved I1 without a valid dcookie");
			    DBG_log("send a dcookie and forget this state"));
			SEND_NOTIFICATION_AA(v2N_COOKIE, &dc);
			return STF_FAIL;
		}
	} else {
		DBG(DBG_CONTROLMORE,
		    DBG_log("will not send/process a dcookie"));
	}
	{
		struct ikev2_ke *ke;
		ke = &md->chain[ISAKMP_NEXT_v2KE]->payload.v2ke;
		st->st_oakley.group = lookup_group(ke->isak_group);
		if (st->st_oakley.group == NULL) {
			char fromname[ADDRTOT_BUF];
			addrtot(&md->sender, 0, fromname, ADDRTOT_BUF);
			libreswan_log(
				"rejecting I1 from %s:%u, invalid DH group=%u",
				fromname, md->sender_port,
				ke->isak_group);
			return v2N_INVALID_KE_PAYLOAD;
		}
	}
	{
		struct ke_continuation *ke = alloc_thing(
			struct ke_continuation,
			"ikev2_inI1outR1 KE");
		stf_status e;
		ke->md = md;
		set_suspended(st, ke->md);
		if (!st->st_sec_in_use) {
			pcrc_init(&ke->ke_pcrc);
			ke->ke_pcrc.pcrc_func =
				ikev2_parent_inI1outR1_continue;
			e = build_ke(&ke->ke_pcrc, st, st->st_oakley.group,
				     pcim_stranger_crypto);
			if (e != STF_SUSPEND && e != STF_INLINE) {
				loglog(RC_CRYPTOFAILED, "system too busy");
				delete_state(st);
			}
		} else {
			e =
				ikev2_parent_inI1outR1_tail((struct
							     pluto_crypto_req_cont
							     *)ke,
							    NULL);
		}
		reset_globals();
		return e;
	}
}