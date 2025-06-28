process_tgs_req(struct server_handle *handle, krb5_data *pkt,
                const krb5_fulladdr *from, krb5_data **response)
{
    krb5_keyblock * subkey = 0;
    krb5_keyblock *header_key = NULL;
    krb5_kdc_req *request = 0;
    krb5_db_entry *server = NULL;
    krb5_db_entry *stkt_server = NULL;
    krb5_kdc_rep reply;
    krb5_enc_kdc_rep_part reply_encpart;
    krb5_ticket ticket_reply, *header_ticket = 0;
    int st_idx = 0;
    krb5_enc_tkt_part enc_tkt_reply;
    int newtransited = 0;
    krb5_error_code retval = 0;
    krb5_keyblock encrypting_key;
    krb5_timestamp kdc_time, authtime = 0;
    krb5_keyblock session_key;
    krb5_keyblock *reply_key = NULL;
    krb5_key_data  *server_key;
    krb5_principal cprinc = NULL, sprinc = NULL, altcprinc = NULL;
    krb5_last_req_entry *nolrarray[2], nolrentry;
    int errcode;
    const char        *status = 0;
    krb5_enc_tkt_part *header_enc_tkt = NULL;  
    krb5_enc_tkt_part *subject_tkt = NULL;  
    krb5_db_entry *client = NULL, *header_server = NULL;
    krb5_db_entry *local_tgt, *local_tgt_storage = NULL;
    krb5_pa_s4u_x509_user *s4u_x509_user = NULL;  
    krb5_authdata **kdc_issued_auth_data = NULL;  
    unsigned int c_flags = 0, s_flags = 0;        
    krb5_boolean is_referral;
    const char *emsg = NULL;
    krb5_kvno ticket_kvno = 0;
    struct kdc_request_state *state = NULL;
    krb5_pa_data *pa_tgs_req;  
    krb5_data scratch;
    krb5_pa_data **e_data = NULL;
    kdc_realm_t *kdc_active_realm = NULL;
    krb5_audit_state *au_state = NULL;
    krb5_data **auth_indicators = NULL;
    memset(&reply, 0, sizeof(reply));
    memset(&reply_encpart, 0, sizeof(reply_encpart));
    memset(&ticket_reply, 0, sizeof(ticket_reply));
    memset(&enc_tkt_reply, 0, sizeof(enc_tkt_reply));
    session_key.contents = NULL;
    retval = decode_krb5_tgs_req(pkt, &request);
    if (retval)
        return retval;
    sprinc = request->server;
    if (request->msg_type != KRB5_TGS_REQ) {
        krb5_free_kdc_req(handle->kdc_err_context, request);
        return KRB5_BADMSGTYPE;
    }
    kdc_active_realm = setup_server_realm(handle, request->server);
    if (kdc_active_realm == NULL) {
        krb5_free_kdc_req(handle->kdc_err_context, request);
        return KRB5KDC_ERR_WRONG_REALM;
    }
    errcode = kdc_make_rstate(kdc_active_realm, &state);
    if (errcode !=0) {
        krb5_free_kdc_req(handle->kdc_err_context, request);
        return errcode;
    }
    errcode = kau_init_kdc_req(kdc_context, request, from, &au_state);
    if (errcode) {
        krb5_free_kdc_req(handle->kdc_err_context, request);
        return errcode;
    }
    kau_tgs_req(kdc_context, TRUE, au_state);
    errcode = kdc_process_tgs_req(kdc_active_realm,
                                  request, from, pkt, &header_ticket,
                                  &header_server, &header_key, &subkey,
                                  &pa_tgs_req);
    if (header_ticket && header_ticket->enc_part2)
        cprinc = header_ticket->enc_part2->client;
    if (errcode) {
        status = "PROCESS_TGS";
        goto cleanup;
    }
    if (!header_ticket) {
        errcode = KRB5_NO_TKT_SUPPLIED;         
        status="UNEXPECTED NULL in header_ticket";
        goto cleanup;
    }
    errcode = kau_make_tkt_id(kdc_context, header_ticket,
                              &au_state->tkt_in_id);
    if (errcode) {
        status = "GENERATE_TICKET_ID";
        goto cleanup;
    }
    scratch.length = pa_tgs_req->length;
    scratch.data = (char *) pa_tgs_req->contents;
    errcode = kdc_find_fast(&request, &scratch, subkey,
                            header_ticket->enc_part2->session, state, NULL);
    sprinc = request->server;
    if (errcode !=0) {
        status = "FIND_FAST";
        goto cleanup;
    }
    errcode = get_local_tgt(kdc_context, &sprinc->realm, header_server,
                            &local_tgt, &local_tgt_storage);
    if (errcode) {
        status = "GET_LOCAL_TGT";
        goto cleanup;
    }
    au_state->request = request;
    header_enc_tkt = header_ticket->enc_part2;
    au_state->stage = SRVC_PRINC;
    setflag(s_flags, KRB5_KDB_FLAG_ALIAS_OK);
    if (isflagset(request->kdc_options, KDC_OPT_CANONICALIZE)) {
        setflag(c_flags, KRB5_KDB_FLAG_CANONICALIZE);
        setflag(s_flags, KRB5_KDB_FLAG_CANONICALIZE);
    }
    errcode = search_sprinc(kdc_active_realm, request, s_flags, &server,
                            &status);
    if (errcode != 0)
        goto cleanup;
    sprinc = server->princ;
    is_referral = is_cross_tgs_principal(server->princ) &&
        !krb5_principal_compare(kdc_context, request->server, server->princ);
    au_state->stage = VALIDATE_POL;
    if ((errcode = krb5_timeofday(kdc_context, &kdc_time))) {
        status = "TIME_OF_DAY";
        goto cleanup;
    }
    if ((retval = validate_tgs_request(kdc_active_realm,
                                       request, *server, header_ticket,
                                       kdc_time, &status, &e_data))) {
        if (!status)
            status = "UNKNOWN_REASON";
        if (retval == KDC_ERR_POLICY || retval == KDC_ERR_BADOPTION)
            au_state->violation = PROT_CONSTRAINT;
        errcode = retval + ERROR_TABLE_BASE_krb5;
        goto cleanup;
    }
    if (!is_local_principal(kdc_active_realm, header_enc_tkt->client))
        setflag(c_flags, KRB5_KDB_FLAG_CROSS_REALM);
    errcode = kdc_process_s4u2self_req(kdc_active_realm,
                                       request,
                                       header_enc_tkt->client,
                                       server,
                                       subkey,
                                       header_enc_tkt->session,
                                       kdc_time,
                                       &s4u_x509_user,
                                       &client,
                                       &status);
    if (s4u_x509_user != NULL || errcode != 0) {
        if (s4u_x509_user != NULL)
            au_state->s4u2self_user = s4u_x509_user->user_id.user;
        if (errcode == KDC_ERR_POLICY || errcode == KDC_ERR_BADOPTION)
            au_state->violation = PROT_CONSTRAINT;
        au_state->status = status;
        kau_s4u2self(kdc_context, errcode ? FALSE : TRUE, au_state);
        au_state->s4u2self_user = NULL;
    }
    if (errcode)
        goto cleanup;
    if (s4u_x509_user != NULL) {
        setflag(c_flags, KRB5_KDB_FLAG_PROTOCOL_TRANSITION);
        if (is_referral) {
            errcode = KRB5KDC_ERR_S_PRINCIPAL_UNKNOWN;
            status = "LOOKING_UP_SERVER";
            goto cleanup;
        }
    }
    errcode = decrypt_2ndtkt(kdc_active_realm, request, c_flags,
                             &stkt_server, &status);
    if (errcode)
        goto cleanup;
    if (isflagset(request->kdc_options, KDC_OPT_CNAME_IN_ADDL_TKT)) {
        errcode = kdc_process_s4u2proxy_req(kdc_active_realm,
                                            request,
                                            request->second_ticket[st_idx]->enc_part2,
                                            stkt_server,
                                            header_ticket->enc_part2->client,
                                            request->server,
                                            &status);
        if (errcode == KDC_ERR_POLICY || errcode == KDC_ERR_BADOPTION)
            au_state->violation = PROT_CONSTRAINT;
        else if (errcode)
            au_state->violation = LOCAL_POLICY;
        au_state->status = status;
        retval = kau_make_tkt_id(kdc_context, request->second_ticket[st_idx],
                                  &au_state->evid_tkt_id);
        if (retval) {
            status = "GENERATE_TICKET_ID";
            errcode = retval;
            goto cleanup;
        }
        kau_s4u2proxy(kdc_context, errcode ? FALSE : TRUE, au_state);
        if (errcode)
            goto cleanup;
        setflag(c_flags, KRB5_KDB_FLAG_CONSTRAINED_DELEGATION);
        assert(krb5_is_tgs_principal(header_ticket->server));
        assert(client == NULL);  
        client = stkt_server;
        stkt_server = NULL;
    } else if (request->kdc_options & KDC_OPT_ENC_TKT_IN_SKEY) {
        krb5_db_free_principal(kdc_context, stkt_server);
        stkt_server = NULL;
    } else
        assert(stkt_server == NULL);
    au_state->stage = ISSUE_TKT;
    errcode = gen_session_key(kdc_active_realm, request, server, &session_key,
                              &status);
    if (errcode)
        goto cleanup;
    if (isflagset(c_flags, KRB5_KDB_FLAG_CONSTRAINED_DELEGATION))
        subject_tkt = request->second_ticket[st_idx]->enc_part2;
    else
        subject_tkt = header_enc_tkt;
    authtime = subject_tkt->times.authtime;
    if (s4u_x509_user == NULL) {
        errcode = get_auth_indicators(kdc_context, subject_tkt, local_tgt,
                                      &auth_indicators);
        if (errcode) {
            status = "GET_AUTH_INDICATORS";
            goto cleanup;
        }
    }
    errcode = check_indicators(kdc_context, server, auth_indicators);
    if (errcode) {
        status = "HIGHER_AUTHENTICATION_REQUIRED";
        goto cleanup;
    }
    if (is_referral)
        ticket_reply.server = server->princ;
    else
        ticket_reply.server = request->server;  
    enc_tkt_reply.flags = OPTS2FLAGS(request->kdc_options);
    enc_tkt_reply.flags |= COPY_TKT_FLAGS(header_enc_tkt->flags);
    enc_tkt_reply.times.starttime = 0;
    if (isflagset(server->attributes, KRB5_KDB_OK_AS_DELEGATE))
        setflag(enc_tkt_reply.flags, TKT_FLG_OK_AS_DELEGATE);
    setflag(enc_tkt_reply.flags, TKT_FLG_ENC_PA_REP);
    enc_tkt_reply.caddrs = header_enc_tkt->caddrs;
    reply_encpart.caddrs = 0; 
    reply_encpart.enc_padata = NULL;
    if (isflagset(request->kdc_options, KDC_OPT_FORWARDABLE)) {
        if (isflagset(c_flags, KRB5_KDB_FLAG_PROTOCOL_TRANSITION)) {
            if (client != NULL &&
                isflagset(client->attributes, KRB5_KDB_DISALLOW_FORWARDABLE))
                clear(enc_tkt_reply.flags, TKT_FLG_FORWARDABLE);
            else if (!isflagset(header_enc_tkt->flags, TKT_FLG_FORWARDABLE))
                clear(enc_tkt_reply.flags, TKT_FLG_FORWARDABLE);
            else if (!is_referral &&
                     !isflagset(server->attributes,
                                KRB5_KDB_OK_TO_AUTH_AS_DELEGATE))
                clear(enc_tkt_reply.flags, TKT_FLG_FORWARDABLE);
        }
    }
    if (isflagset(request->kdc_options, KDC_OPT_FORWARDED) ||
        isflagset(request->kdc_options, KDC_OPT_PROXY)) {
        enc_tkt_reply.caddrs = request->addresses;
        reply_encpart.caddrs = request->addresses;
    }
    if (isflagset(request->kdc_options, KDC_OPT_REQUEST_ANONYMOUS) &&
        !isflagset(header_enc_tkt->flags, TKT_FLG_ANONYMOUS))
        clear(enc_tkt_reply.flags, TKT_FLG_ANONYMOUS);
    if (isflagset(request->kdc_options, KDC_OPT_POSTDATED)) {
        setflag(enc_tkt_reply.flags, TKT_FLG_INVALID);
        enc_tkt_reply.times.starttime = request->from;
    } else
        enc_tkt_reply.times.starttime = kdc_time;
    if (isflagset(request->kdc_options, KDC_OPT_VALIDATE)) {
        assert(isflagset(c_flags, KRB5_KDB_FLAGS_S4U) == 0);
        ticket_reply = *(header_ticket);
        enc_tkt_reply = *(header_ticket->enc_part2);
        enc_tkt_reply.authorization_data = NULL;
        clear(enc_tkt_reply.flags, TKT_FLG_INVALID);
    }
    if (isflagset(request->kdc_options, KDC_OPT_RENEW)) {
        krb5_timestamp old_starttime;
        krb5_deltat old_life;
        assert(isflagset(c_flags, KRB5_KDB_FLAGS_S4U) == 0);
        ticket_reply = *(header_ticket);
        enc_tkt_reply = *(header_ticket->enc_part2);
        enc_tkt_reply.authorization_data = NULL;
        old_starttime = enc_tkt_reply.times.starttime ?
            enc_tkt_reply.times.starttime : enc_tkt_reply.times.authtime;
        old_life = ts_delta(enc_tkt_reply.times.endtime, old_starttime);
        enc_tkt_reply.times.starttime = kdc_time;
        enc_tkt_reply.times.endtime =
            ts_min(header_ticket->enc_part2->times.renew_till,
                   ts_incr(kdc_time, old_life));
    } else {
        enc_tkt_reply.times.starttime = kdc_time;
        kdc_get_ticket_endtime(kdc_active_realm, enc_tkt_reply.times.starttime,
                               header_enc_tkt->times.endtime, request->till,
                               client, server, &enc_tkt_reply.times.endtime);
    }
    kdc_get_ticket_renewtime(kdc_active_realm, request, header_enc_tkt, client,
                             server, &enc_tkt_reply);
    enc_tkt_reply.times.authtime = authtime;
    if (enc_tkt_reply.times.starttime == enc_tkt_reply.times.authtime)
        enc_tkt_reply.times.starttime = 0;
    if (isflagset(c_flags, KRB5_KDB_FLAG_PROTOCOL_TRANSITION)) {
        altcprinc = s4u_x509_user->user_id.user;
    } else if (isflagset(c_flags, KRB5_KDB_FLAG_CONSTRAINED_DELEGATION)) {
        altcprinc = subject_tkt->client;
    } else {
        altcprinc = NULL;
    }
    if (isflagset(request->kdc_options, KDC_OPT_ENC_TKT_IN_SKEY)) {
        krb5_enc_tkt_part *t2enc = request->second_ticket[st_idx]->enc_part2;
        encrypting_key = *(t2enc->session);
    } else {
        if ((errcode = krb5_dbe_find_enctype(kdc_context, server,
                                             -1,  
                                             -1,  
                                             0,   
                                             &server_key))) {
            status = "FINDING_SERVER_KEY";
            goto cleanup;
        }
        if ((errcode = krb5_dbe_decrypt_key_data(kdc_context, NULL,
                                                 server_key, &encrypting_key,
                                                 NULL))) {
            status = "DECRYPT_SERVER_KEY";
            goto cleanup;
        }
    }
    if (isflagset(c_flags, KRB5_KDB_FLAG_CONSTRAINED_DELEGATION)) {
        clear(server->attributes, KRB5_KDB_NO_AUTH_DATA_REQUIRED);
    }
    if (isflagset(server->attributes, KRB5_KDB_NO_AUTH_DATA_REQUIRED) == 0) {
        if (!isflagset(c_flags, KRB5_KDB_FLAGS_S4U)) {
            setflag(c_flags, KRB5_KDB_FLAG_INCLUDE_PAC);
            setflag(c_flags, KRB5_KDB_FLAG_MAP_PRINCIPALS);
            assert(client == NULL);  
            errcode = krb5_db_get_principal(kdc_context, subject_tkt->client,
                                            c_flags, &client);
        }
    }
    if (isflagset(c_flags, KRB5_KDB_FLAG_PROTOCOL_TRANSITION) &&
        !isflagset(c_flags, KRB5_KDB_FLAG_CROSS_REALM))
        enc_tkt_reply.client = s4u_x509_user->user_id.user;
    else
        enc_tkt_reply.client = subject_tkt->client;
    enc_tkt_reply.session = &session_key;
    enc_tkt_reply.transited.tr_type = KRB5_DOMAIN_X500_COMPRESS;
    enc_tkt_reply.transited.tr_contents = empty_string;  
    if (krb5_realm_compare(kdc_context, header_ticket->server, tgs_server) ||
        krb5_realm_compare(kdc_context, header_ticket->server,
                           enc_tkt_reply.client)) {
        enc_tkt_reply.transited = header_enc_tkt->transited;
    } else {
        if (header_enc_tkt->transited.tr_type !=
            KRB5_DOMAIN_X500_COMPRESS) {
            status = "VALIDATE_TRANSIT_TYPE";
            errcode = KRB5KDC_ERR_TRTYPE_NOSUPP;
            goto cleanup;
        }
        memset(&enc_tkt_reply.transited, 0, sizeof(enc_tkt_reply.transited));
        enc_tkt_reply.transited.tr_type = KRB5_DOMAIN_X500_COMPRESS;
        if ((errcode =
             add_to_transited(&header_enc_tkt->transited.tr_contents,
                              &enc_tkt_reply.transited.tr_contents,
                              header_ticket->server,
                              enc_tkt_reply.client,
                              request->server))) {
            status = "ADD_TO_TRANSITED_LIST";
            goto cleanup;
        }
        newtransited = 1;
    }
    if (isflagset(c_flags, KRB5_KDB_FLAG_CROSS_REALM)) {
        errcode = validate_transit_path(kdc_context, header_enc_tkt->client,
                                        server, header_server);
        if (errcode) {
            status = "NON_TRANSITIVE";
            goto cleanup;
        }
    }
    if (!isflagset (request->kdc_options, KDC_OPT_DISABLE_TRANSITED_CHECK)) {
        errcode = kdc_check_transited_list (kdc_active_realm,
                                            &enc_tkt_reply.transited.tr_contents,
                                            krb5_princ_realm (kdc_context, header_enc_tkt->client),
                                            krb5_princ_realm (kdc_context, request->server));
        if (errcode == 0) {
            setflag (enc_tkt_reply.flags, TKT_FLG_TRANSIT_POLICY_CHECKED);
        } else {
            log_tgs_badtrans(kdc_context, cprinc, sprinc,
                             &enc_tkt_reply.transited.tr_contents, errcode);
        }
    } else
        krb5_klog_syslog(LOG_INFO, _("not checking transit path"));
    if (kdc_active_realm->realm_reject_bad_transit &&
        !isflagset(enc_tkt_reply.flags, TKT_FLG_TRANSIT_POLICY_CHECKED)) {
        errcode = KRB5KDC_ERR_POLICY;
        status = "BAD_TRANSIT";
        au_state->violation = LOCAL_POLICY;
        goto cleanup;
    }
    errcode = handle_authdata(kdc_context, c_flags, client, server,
                              header_server, local_tgt,
                              subkey != NULL ? subkey :
                              header_ticket->enc_part2->session,
                              &encrypting_key,  
                              header_key,
                              pkt,
                              request,
                              s4u_x509_user ?
                              s4u_x509_user->user_id.user : NULL,
                              subject_tkt,
                              auth_indicators,
                              &enc_tkt_reply);
    if (errcode) {
        krb5_klog_syslog(LOG_INFO, _("TGS_REQ : handle_authdata (%d)"),
                         errcode);
        status = "HANDLE_AUTHDATA";
        goto cleanup;
    }
    ticket_reply.enc_part2 = &enc_tkt_reply;
    if (isflagset(request->kdc_options, KDC_OPT_ENC_TKT_IN_SKEY)) {
        krb5_enc_tkt_part *t2enc = request->second_ticket[st_idx]->enc_part2;
        krb5_principal client2 = t2enc->client;
        if (!krb5_principal_compare(kdc_context, request->server, client2)) {
            altcprinc = client2;
            errcode = KRB5KDC_ERR_SERVER_NOMATCH;
            status = "2ND_TKT_MISMATCH";
            au_state->status = status;
            kau_u2u(kdc_context, FALSE, au_state);
            goto cleanup;
        }
        ticket_kvno = 0;
        ticket_reply.enc_part.enctype = t2enc->session->enctype;
        kau_u2u(kdc_context, TRUE, au_state);
        st_idx++;
    } else {
        ticket_kvno = server_key->key_data_kvno;
    }
    errcode = krb5_encrypt_tkt_part(kdc_context, &encrypting_key,
                                    &ticket_reply);
    if (!isflagset(request->kdc_options, KDC_OPT_ENC_TKT_IN_SKEY))
        krb5_free_keyblock_contents(kdc_context, &encrypting_key);
    if (errcode) {
        status = "ENCRYPT_TICKET";
        goto cleanup;
    }
    ticket_reply.enc_part.kvno = ticket_kvno;
    au_state->stage = ENCR_REP;
    reply.msg_type = KRB5_TGS_REP;
    if (isflagset(c_flags, KRB5_KDB_FLAG_PROTOCOL_TRANSITION) &&
        krb5int_find_pa_data(kdc_context, request->padata,
                             KRB5_PADATA_S4U_X509_USER) != NULL) {
        errcode = kdc_make_s4u2self_rep(kdc_context,
                                        subkey,
                                        header_ticket->enc_part2->session,
                                        s4u_x509_user,
                                        &reply,
                                        &reply_encpart);
        if (errcode) {
            status = "MAKE_S4U2SELF_PADATA";
            au_state->status = status;
        }
        kau_s4u2self(kdc_context, errcode ? FALSE : TRUE, au_state);
        if (errcode)
            goto cleanup;
    }
    reply.client = enc_tkt_reply.client;
    reply.enc_part.kvno = 0; 
    reply.ticket = &ticket_reply;
    reply_encpart.session = &session_key;
    reply_encpart.nonce = request->nonce;
    reply_encpart.times = enc_tkt_reply.times;
    nolrentry.lr_type = KRB5_LRQ_NONE;
    nolrentry.value = 0;
    nolrentry.magic = 0;
    nolrarray[0] = &nolrentry;
    nolrarray[1] = 0;
    reply_encpart.last_req = nolrarray;         
    reply_encpart.key_exp = 0; 
    reply_encpart.flags = enc_tkt_reply.flags;
    reply_encpart.server = ticket_reply.server;
    reply.enc_part.enctype = subkey ? subkey->enctype :
        header_ticket->enc_part2->session->enctype;
    errcode  = kdc_fast_response_handle_padata(state, request, &reply,
                                               subkey ? subkey->enctype : header_ticket->enc_part2->session->enctype);
    if (errcode !=0 ) {
        status = "MAKE_FAST_RESPONSE";
        goto cleanup;
    }
    errcode =kdc_fast_handle_reply_key(state,
                                       subkey?subkey:header_ticket->enc_part2->session, &reply_key);
    if (errcode) {
        status  = "MAKE_FAST_REPLY_KEY";
        goto cleanup;
    }
    errcode = return_enc_padata(kdc_context, pkt, request,
                                reply_key, server, &reply_encpart,
                                is_referral &&
                                isflagset(s_flags,
                                          KRB5_KDB_FLAG_CANONICALIZE));
    if (errcode) {
        status = "KDC_RETURN_ENC_PADATA";
        goto cleanup;
    }
    errcode = kau_make_tkt_id(kdc_context, &ticket_reply, &au_state->tkt_out_id);
    if (errcode) {
        status = "GENERATE_TICKET_ID";
        goto cleanup;
    }
    if (kdc_fast_hide_client(state))
        reply.client = (krb5_principal)krb5_anonymous_principal();
    errcode = krb5_encode_kdc_rep(kdc_context, KRB5_TGS_REP, &reply_encpart,
                                  subkey ? 1 : 0,
                                  reply_key,
                                  &reply, response);
    if (errcode) {
        status = "ENCODE_KDC_REP";
    } else {
        status = "ISSUE";
    }
    memset(ticket_reply.enc_part.ciphertext.data, 0,
           ticket_reply.enc_part.ciphertext.length);
    free(ticket_reply.enc_part.ciphertext.data);
    memset(reply.enc_part.ciphertext.data, 0,
           reply.enc_part.ciphertext.length);
    free(reply.enc_part.ciphertext.data);
cleanup:
    assert(status != NULL);
    if (reply_key)
        krb5_free_keyblock(kdc_context, reply_key);
    if (errcode)
        emsg = krb5_get_error_message (kdc_context, errcode);
    au_state->status = status;
    if (!errcode)
        au_state->reply = &reply;
    kau_tgs_req(kdc_context, errcode ? FALSE : TRUE, au_state);
    kau_free_kdc_req(au_state);
    log_tgs_req(kdc_context, from, request, &reply, cprinc,
                sprinc, altcprinc, authtime,
                c_flags, status, errcode, emsg);
    if (errcode) {
        krb5_free_error_message (kdc_context, emsg);
        emsg = NULL;
    }
    if (errcode) {
        int got_err = 0;
        if (status == 0) {
            status = krb5_get_error_message (kdc_context, errcode);
            got_err = 1;
        }
        errcode -= ERROR_TABLE_BASE_krb5;
        if (errcode < 0 || errcode > KRB_ERR_MAX)
            errcode = KRB_ERR_GENERIC;
        retval = prepare_error_tgs(state, request, header_ticket, errcode,
                                   (server != NULL) ? server->princ : NULL,
                                   response, status, e_data);
        if (got_err) {
            krb5_free_error_message (kdc_context, status);
            status = 0;
        }
    }
    if (header_ticket != NULL)
        krb5_free_ticket(kdc_context, header_ticket);
    if (request != NULL)
        krb5_free_kdc_req(kdc_context, request);
    if (state)
        kdc_free_rstate(state);
    krb5_db_free_principal(kdc_context, server);
    krb5_db_free_principal(kdc_context, stkt_server);
    krb5_db_free_principal(kdc_context, header_server);
    krb5_db_free_principal(kdc_context, client);
    krb5_db_free_principal(kdc_context, local_tgt_storage);
    if (session_key.contents != NULL)
        krb5_free_keyblock_contents(kdc_context, &session_key);
    if (newtransited)
        free(enc_tkt_reply.transited.tr_contents.data);
    if (s4u_x509_user != NULL)
        krb5_free_pa_s4u_x509_user(kdc_context, s4u_x509_user);
    if (kdc_issued_auth_data != NULL)
        krb5_free_authdata(kdc_context, kdc_issued_auth_data);
    if (subkey != NULL)
        krb5_free_keyblock(kdc_context, subkey);
    if (header_key != NULL)
        krb5_free_keyblock(kdc_context, header_key);
    if (reply.padata)
        krb5_free_pa_data(kdc_context, reply.padata);
    if (reply_encpart.enc_padata)
        krb5_free_pa_data(kdc_context, reply_encpart.enc_padata);
    if (enc_tkt_reply.authorization_data != NULL)
        krb5_free_authdata(kdc_context, enc_tkt_reply.authorization_data);
    krb5_free_pa_data(kdc_context, e_data);
    k5_free_data_ptr_list(auth_indicators);
    return retval;
}