tgs_issue_ticket(kdc_realm_t *realm, struct tgs_req_info *t,
                 krb5_flags tktflags, krb5_ticket_times *times, krb5_data *pkt,
                 const krb5_fulladdr *from,
                 struct kdc_request_state *fast_state,
                 krb5_audit_state *au_state, const char **status,
                 krb5_data **response)
{
    krb5_context context = realm->realm_context;
    krb5_error_code ret;
    krb5_keyblock session_key = { 0 }, server_key = { 0 };
    krb5_keyblock *ticket_encrypting_key, *subject_key;
    krb5_keyblock *initial_reply_key, *fast_reply_key = NULL;
    krb5_enc_tkt_part enc_tkt_reply = { 0 };
    krb5_ticket ticket_reply = { 0 };
    krb5_enc_kdc_rep_part reply_encpart = { 0 };
    krb5_kdc_rep reply = { 0 };
    krb5_pac subject_pac;
    krb5_db_entry *subject_server;
    krb5_enc_tkt_part *header_enc_tkt = t->header_tkt->enc_part2;
    krb5_last_req_entry nolrentry = { KV5M_LAST_REQ_ENTRY, KRB5_LRQ_NONE, 0 };
    krb5_last_req_entry *nolrarray[2] = { &nolrentry, NULL };
    au_state->stage = ISSUE_TKT;
    ret = gen_session_key(context, t->req, t->server, &session_key, status);
    if (ret)
        goto cleanup;
    if (t->flags & KRB5_KDB_FLAG_CONSTRAINED_DELEGATION) {
        subject_pac = t->stkt_pac;
        subject_server = t->stkt_server;
        subject_key = t->stkt_server_key;
    } else {
        subject_pac = t->header_pac;
        subject_server = t->header_server;
        subject_key = t->header_key;
    }
    initial_reply_key = (t->subkey != NULL) ? t->subkey :
        t->header_tkt->enc_part2->session;
    if (t->req->kdc_options & KDC_OPT_ENC_TKT_IN_SKEY) {
        ticket_encrypting_key = t->stkt->enc_part2->session;
    } else {
        ret = get_first_current_key(context, t->server, &server_key);
        if (ret) {
            *status = "FINDING_SERVER_KEY";
            goto cleanup;
        }
        ticket_encrypting_key = &server_key;
    }
    if (t->req->kdc_options & (KDC_OPT_VALIDATE | KDC_OPT_RENEW)) {
        ticket_reply = *t->header_tkt;
        enc_tkt_reply = *t->header_tkt->enc_part2;
        enc_tkt_reply.authorization_data = NULL;
    } else {
        if (t->req->kdc_options & (KDC_OPT_FORWARDED | KDC_OPT_PROXY)) {
            enc_tkt_reply.caddrs = t->req->addresses;
            reply_encpart.caddrs = t->req->addresses;
        } else {
            enc_tkt_reply.caddrs = header_enc_tkt->caddrs;
            reply_encpart.caddrs = NULL;
        }
        ticket_reply.server = t->is_referral ? t->sprinc : t->req->server;
    }
    enc_tkt_reply.flags = tktflags;
    enc_tkt_reply.times = *times;
    enc_tkt_reply.client = t->tkt_client;
    enc_tkt_reply.session = &session_key;
    enc_tkt_reply.transited = t->transited;
    ret = handle_authdata(realm, t->flags, t->client, t->server,
                          subject_server, t->local_tgt, &t->local_tgt_key,
                          initial_reply_key, ticket_encrypting_key,
                          subject_key, NULL, pkt, t->req, t->s4u_cprinc,
                          subject_pac, t->subject_tkt, &t->auth_indicators,
                          &enc_tkt_reply);
    if (ret) {
        krb5_klog_syslog(LOG_INFO, _("TGS_REQ : handle_authdata (%d)"), ret);
        *status = "HANDLE_AUTHDATA";
        goto cleanup;
    }
    ticket_reply.enc_part2 = &enc_tkt_reply;
    ret = krb5_encrypt_tkt_part(context, ticket_encrypting_key, &ticket_reply);
    if (ret)
        goto cleanup;
    if (t->req->kdc_options & KDC_OPT_ENC_TKT_IN_SKEY) {
        ticket_reply.enc_part.kvno = 0;
        kau_u2u(context, TRUE, au_state);
    } else {
        ticket_reply.enc_part.kvno = current_kvno(t->server);
    }
    au_state->stage = ENCR_REP;
    if (t->s4u2self != NULL &&
        krb5int_find_pa_data(context, t->req->padata,
                             KRB5_PADATA_S4U_X509_USER) != NULL) {
        ret = kdc_make_s4u2self_rep(context, t->subkey,
                                    t->header_tkt->enc_part2->session,
                                    t->s4u2self, &reply, &reply_encpart);
        if (ret)
            goto cleanup;
    }
    reply_encpart.session = &session_key;
    reply_encpart.nonce = t->req->nonce;
    reply_encpart.times = enc_tkt_reply.times;
    reply_encpart.last_req = nolrarray;
    reply_encpart.key_exp = 0;
    reply_encpart.flags = enc_tkt_reply.flags;
    reply_encpart.server = ticket_reply.server;
    reply.msg_type = KRB5_TGS_REP;
    reply.client = enc_tkt_reply.client;
    reply.ticket = &ticket_reply;
    reply.enc_part.kvno = 0;
    reply.enc_part.enctype = initial_reply_key->enctype;
    ret = kdc_fast_response_handle_padata(fast_state, t->req, &reply,
                                          initial_reply_key->enctype);
    if (ret)
        goto cleanup;
    ret = kdc_fast_handle_reply_key(fast_state, initial_reply_key,
                                    &fast_reply_key);
    if (ret)
        goto cleanup;
    ret = return_enc_padata(context, pkt, t->req, fast_reply_key, t->server,
                            &reply_encpart,
                            t->is_referral &&
                            (t->req->kdc_options & KDC_OPT_CANONICALIZE));
    if (ret) {
        *status = "KDC_RETURN_ENC_PADATA";
        goto cleanup;
    }
    ret = kau_make_tkt_id(context, &ticket_reply, &au_state->tkt_out_id);
    if (ret)
        goto cleanup;
    if (kdc_fast_hide_client(fast_state))
        reply.client = (krb5_principal)krb5_anonymous_principal();
    ret = krb5_encode_kdc_rep(context, KRB5_TGS_REP, &reply_encpart,
                              t->subkey != NULL, fast_reply_key, &reply,
                              response);
    if (ret)
        goto cleanup;
    log_tgs_req(context, from, t->req, &reply, t->cprinc, t->sprinc,
                t->s4u_cprinc, t->authtime, t->flags, "ISSUE", 0, NULL);
    au_state->status = "ISSUE";
    au_state->reply = &reply;
    if (t->flags & KRB5_KDB_FLAG_CONSTRAINED_DELEGATION)
        kau_s4u2proxy(context, TRUE, au_state);
    kau_tgs_req(context, TRUE, au_state);
    au_state->reply = NULL;
cleanup:
    zapfree(ticket_reply.enc_part.ciphertext.data,
            ticket_reply.enc_part.ciphertext.length);
    zapfree(reply.enc_part.ciphertext.data, reply.enc_part.ciphertext.length);
    krb5_free_pa_data(context, reply.padata);
    krb5_free_pa_data(context, reply_encpart.enc_padata);
    krb5_free_authdata(context, enc_tkt_reply.authorization_data);
    krb5_free_keyblock_contents(context, &session_key);
    krb5_free_keyblock_contents(context, &server_key);
    krb5_free_keyblock(context, fast_reply_key);
    return ret;
}