int ssl3_accept(SSL *s)
	{
	BUF_MEM *buf;
	unsigned long alg_k,Time=(unsigned long)time(NULL);
	void (*cb)(const SSL *ssl,int type,int val)=NULL;
	int ret= -1;
	int new_state,state,skip=0;
	RAND_add(&Time,sizeof(Time),0);
	ERR_clear_error();
	clear_sys_error();
	if (s->info_callback != NULL)
		cb=s->info_callback;
	else if (s->ctx->info_callback != NULL)
		cb=s->ctx->info_callback;
	s->in_handshake++;
	if (!SSL_in_init(s) || SSL_in_before(s)) SSL_clear(s);
	if (s->cert == NULL)
		{
		SSLerr(SSL_F_SSL3_ACCEPT,SSL_R_NO_CERTIFICATE_SET);
		return(-1);
		}
#ifndef OPENSSL_NO_HEARTBEATS
	if (s->tlsext_hb_pending)
		{
		s->tlsext_hb_pending = 0;
		s->tlsext_hb_seq++;
		}
#endif
	for (;;)
		{
		state=s->state;
		switch (s->state)
			{
		case SSL_ST_RENEGOTIATE:
			s->renegotiate=1;
		case SSL_ST_BEFORE:
		case SSL_ST_ACCEPT:
		case SSL_ST_BEFORE|SSL_ST_ACCEPT:
		case SSL_ST_OK|SSL_ST_ACCEPT:
			s->server=1;
			if (cb != NULL) cb(s,SSL_CB_HANDSHAKE_START,1);
			if ((s->version>>8) != 3)
				{
				SSLerr(SSL_F_SSL3_ACCEPT, ERR_R_INTERNAL_ERROR);
				return -1;
				}
			if (!ssl_security(s, SSL_SECOP_VERSION, 0,
							s->version, NULL))
				{
				SSLerr(SSL_F_SSL3_ACCEPT, SSL_R_VERSION_TOO_LOW);
				return -1;
				}
			s->type=SSL_ST_ACCEPT;
			if (s->init_buf == NULL)
				{
				if ((buf=BUF_MEM_new()) == NULL)
					{
					ret= -1;
					goto end;
					}
				if (!BUF_MEM_grow(buf,SSL3_RT_MAX_PLAIN_LENGTH))
					{
					BUF_MEM_free(buf);
					ret= -1;
					goto end;
					}
				s->init_buf=buf;
				}
			if (!ssl3_setup_buffers(s))
				{
				ret= -1;
				goto end;
				}
			s->init_num=0;
			s->s3->flags &= ~TLS1_FLAGS_SKIP_CERT_VERIFY;
			s->s3->flags &= ~SSL3_FLAGS_CCS_OK;
			s->s3->change_cipher_spec = 0;
			if (s->state != SSL_ST_RENEGOTIATE)
				{
				if (!ssl_init_wbio_buffer(s,1)) { ret= -1; goto end; }
				ssl3_init_finished_mac(s);
				s->state=SSL3_ST_SR_CLNT_HELLO_A;
				s->ctx->stats.sess_accept++;
				}
			else if (!s->s3->send_connection_binding &&
				!(s->options & SSL_OP_ALLOW_UNSAFE_LEGACY_RENEGOTIATION))
				{
				SSLerr(SSL_F_SSL3_ACCEPT, SSL_R_UNSAFE_LEGACY_RENEGOTIATION_DISABLED);
				ssl3_send_alert(s,SSL3_AL_FATAL,SSL_AD_HANDSHAKE_FAILURE);
				ret = -1;
				goto end;
				}
			else
				{
				s->ctx->stats.sess_accept_renegotiate++;
				s->state=SSL3_ST_SW_HELLO_REQ_A;
				}
			break;
		case SSL3_ST_SW_HELLO_REQ_A:
		case SSL3_ST_SW_HELLO_REQ_B:
			s->shutdown=0;
			ret=ssl3_send_hello_request(s);
			if (ret <= 0) goto end;
			s->s3->tmp.next_state=SSL3_ST_SW_HELLO_REQ_C;
			s->state=SSL3_ST_SW_FLUSH;
			s->init_num=0;
			ssl3_init_finished_mac(s);
			break;
		case SSL3_ST_SW_HELLO_REQ_C:
			s->state=SSL_ST_OK;
			break;
		case SSL3_ST_SR_CLNT_HELLO_A:
		case SSL3_ST_SR_CLNT_HELLO_B:
		case SSL3_ST_SR_CLNT_HELLO_C:
			ret=ssl3_get_client_hello(s);
			if (ret <= 0) goto end;
#ifndef OPENSSL_NO_SRP
			s->state = SSL3_ST_SR_CLNT_HELLO_D;
		case SSL3_ST_SR_CLNT_HELLO_D:
			{
			int al;
			if ((ret = ssl_check_srp_ext_ClientHello(s,&al))  < 0)
					{
					s->rwstate=SSL_X509_LOOKUP;
					goto end;
					}
			if (ret != SSL_ERROR_NONE)
				{
				ssl3_send_alert(s,SSL3_AL_FATAL,al);	
 				   if (al != TLS1_AD_UNKNOWN_PSK_IDENTITY) 	
					SSLerr(SSL_F_SSL3_ACCEPT,SSL_R_CLIENTHELLO_TLSEXT);			
				ret = SSL_TLSEXT_ERR_ALERT_FATAL;			
				ret= -1;
				goto end;	
				}
			}
#endif		
			s->renegotiate = 2;
			s->state=SSL3_ST_SW_SRVR_HELLO_A;
			s->init_num=0;
			break;
		case SSL3_ST_SW_SRVR_HELLO_A:
		case SSL3_ST_SW_SRVR_HELLO_B:
			ret=ssl3_send_server_hello(s);
			if (ret <= 0) goto end;
#ifndef OPENSSL_NO_TLSEXT
			if (s->hit)
				{
				if (s->tlsext_ticket_expected)
					s->state=SSL3_ST_SW_SESSION_TICKET_A;
				else
					s->state=SSL3_ST_SW_CHANGE_A;
				}
#else
			if (s->hit)
					s->state=SSL3_ST_SW_CHANGE_A;
#endif
			else
					s->state = SSL3_ST_SW_CERT_A;
			s->init_num = 0;
			break;
		case SSL3_ST_SW_CERT_A:
		case SSL3_ST_SW_CERT_B:
			if (!(s->s3->tmp.new_cipher->algorithm_auth & (SSL_aNULL|SSL_aKRB5|SSL_aSRP))
				&& !(s->s3->tmp.new_cipher->algorithm_mkey & SSL_kPSK))
				{
				ret=ssl3_send_server_certificate(s);
				if (ret <= 0) goto end;
#ifndef OPENSSL_NO_TLSEXT
				if (s->tlsext_status_expected)
					s->state=SSL3_ST_SW_CERT_STATUS_A;
				else
					s->state=SSL3_ST_SW_KEY_EXCH_A;
				}
			else
				{
				skip = 1;
				s->state=SSL3_ST_SW_KEY_EXCH_A;
				}
#else
				}
			else
				skip=1;
			s->state=SSL3_ST_SW_KEY_EXCH_A;
#endif
			s->init_num=0;
			break;
		case SSL3_ST_SW_KEY_EXCH_A:
		case SSL3_ST_SW_KEY_EXCH_B:
			alg_k = s->s3->tmp.new_cipher->algorithm_mkey;
			if ((s->options & SSL_OP_EPHEMERAL_RSA)
#ifndef OPENSSL_NO_KRB5
				&& !(alg_k & SSL_kKRB5)
#endif  
				)
				s->s3->tmp.use_rsa_tmp=1;
			else
				s->s3->tmp.use_rsa_tmp=0;
			if (s->s3->tmp.use_rsa_tmp
#ifndef OPENSSL_NO_PSK
			    || ((alg_k & SSL_kPSK) && s->ctx->psk_identity_hint)
#endif
#ifndef OPENSSL_NO_SRP
			    || (alg_k & SSL_kSRP)
#endif
			    || (alg_k & SSL_kDHE)
			    || (alg_k & SSL_kECDHE)
			    || ((alg_k & SSL_kRSA)
				&& (s->cert->pkeys[SSL_PKEY_RSA_ENC].privatekey == NULL
				    || (SSL_C_IS_EXPORT(s->s3->tmp.new_cipher)
					&& EVP_PKEY_size(s->cert->pkeys[SSL_PKEY_RSA_ENC].privatekey)*8 > SSL_C_EXPORT_PKEYLENGTH(s->s3->tmp.new_cipher)
					)
				    )
				)
			    )
				{
				ret=ssl3_send_server_key_exchange(s);
				if (ret <= 0) goto end;
				}
			else
				skip=1;
			s->state=SSL3_ST_SW_CERT_REQ_A;
			s->init_num=0;
			break;
		case SSL3_ST_SW_CERT_REQ_A:
		case SSL3_ST_SW_CERT_REQ_B:
			if ( 
				!(s->verify_mode & SSL_VERIFY_PEER) ||
				((s->session->peer != NULL) &&
				 (s->verify_mode & SSL_VERIFY_CLIENT_ONCE)) ||
				((s->s3->tmp.new_cipher->algorithm_auth & SSL_aNULL) &&
				 !(s->verify_mode & SSL_VERIFY_FAIL_IF_NO_PEER_CERT)) ||
				(s->s3->tmp.new_cipher->algorithm_auth & SSL_aKRB5) ||
				(s->s3->tmp.new_cipher->algorithm_auth & SSL_aSRP)
				|| (s->s3->tmp.new_cipher->algorithm_mkey & SSL_kPSK))
				{
				skip=1;
				s->s3->tmp.cert_request=0;
				s->state=SSL3_ST_SW_SRVR_DONE_A;
				if (s->s3->handshake_buffer)
					if (!ssl3_digest_cached_records(s))
						return -1;
				}
			else
				{
				s->s3->tmp.cert_request=1;
				ret=ssl3_send_certificate_request(s);
				if (ret <= 0) goto end;
#ifndef NETSCAPE_HANG_BUG
				s->state=SSL3_ST_SW_SRVR_DONE_A;
#else
				s->state=SSL3_ST_SW_FLUSH;
				s->s3->tmp.next_state=SSL3_ST_SR_CERT_A;
#endif
				s->init_num=0;
				}
			break;
		case SSL3_ST_SW_SRVR_DONE_A:
		case SSL3_ST_SW_SRVR_DONE_B:
			ret=ssl3_send_server_done(s);
			if (ret <= 0) goto end;
			s->s3->tmp.next_state=SSL3_ST_SR_CERT_A;
			s->state=SSL3_ST_SW_FLUSH;
			s->init_num=0;
			break;
		case SSL3_ST_SW_FLUSH:
			s->rwstate=SSL_WRITING;
			if (BIO_flush(s->wbio) <= 0)
				{
				ret= -1;
				goto end;
				}
			s->rwstate=SSL_NOTHING;
			s->state=s->s3->tmp.next_state;
			break;
		case SSL3_ST_SR_CERT_A:
		case SSL3_ST_SR_CERT_B:
			if (s->s3->tmp.cert_request)
				{
				ret=ssl3_get_client_certificate(s);
				if (ret <= 0) goto end;
				}
			s->init_num=0;
			s->state=SSL3_ST_SR_KEY_EXCH_A;
			break;
		case SSL3_ST_SR_KEY_EXCH_A:
		case SSL3_ST_SR_KEY_EXCH_B:
			ret=ssl3_get_client_key_exchange(s);
			if (ret <= 0)
				goto end;
			if (ret == 2)
				{
#if defined(OPENSSL_NO_TLSEXT) || defined(OPENSSL_NO_NEXTPROTONEG)
				s->state=SSL3_ST_SR_FINISHED_A;
#else
				if (s->s3->next_proto_neg_seen)
					s->state=SSL3_ST_SR_NEXT_PROTO_A;
				else
					s->state=SSL3_ST_SR_FINISHED_A;
#endif
				s->init_num = 0;
				}
			else if (SSL_USE_SIGALGS(s))
				{
				s->state=SSL3_ST_SR_CERT_VRFY_A;
				s->init_num=0;
				if (!s->session->peer)
					break;
				if (!s->s3->handshake_buffer)
					{
					SSLerr(SSL_F_SSL3_ACCEPT,ERR_R_INTERNAL_ERROR);
					return -1;
					}
				s->s3->flags |= TLS1_FLAGS_KEEP_HANDSHAKE;
				if (!ssl3_digest_cached_records(s))
					return -1;
				}
			else
				{
				int offset=0;
				int dgst_num;
				s->state=SSL3_ST_SR_CERT_VRFY_A;
				s->init_num=0;
				if (s->s3->handshake_buffer)
					if (!ssl3_digest_cached_records(s))
						return -1;
				for (dgst_num=0; dgst_num<SSL_MAX_DIGEST;dgst_num++)	
					if (s->s3->handshake_dgst[dgst_num]) 
						{
						int dgst_size;
						s->method->ssl3_enc->cert_verify_mac(s,EVP_MD_CTX_type(s->s3->handshake_dgst[dgst_num]),&(s->s3->tmp.cert_verify_md[offset]));
						dgst_size=EVP_MD_CTX_size(s->s3->handshake_dgst[dgst_num]);
						if (dgst_size < 0)
							{
							ret = -1;
							goto end;
							}
						offset+=dgst_size;
						}		
				}
			break;
		case SSL3_ST_SR_CERT_VRFY_A:
		case SSL3_ST_SR_CERT_VRFY_B:
			if (!s->s3->change_cipher_spec)
				s->s3->flags |= SSL3_FLAGS_CCS_OK;
			ret=ssl3_get_cert_verify(s);
			if (ret <= 0) goto end;
#if defined(OPENSSL_NO_TLSEXT) || defined(OPENSSL_NO_NEXTPROTONEG)
			s->state=SSL3_ST_SR_FINISHED_A;
#else
			if (s->s3->next_proto_neg_seen)
				s->state=SSL3_ST_SR_NEXT_PROTO_A;
			else
				s->state=SSL3_ST_SR_FINISHED_A;
#endif
			s->init_num=0;
			break;
#if !defined(OPENSSL_NO_TLSEXT) && !defined(OPENSSL_NO_NEXTPROTONEG)
		case SSL3_ST_SR_NEXT_PROTO_A:
		case SSL3_ST_SR_NEXT_PROTO_B:
			if (!s->s3->change_cipher_spec)
				s->s3->flags |= SSL3_FLAGS_CCS_OK;
			ret=ssl3_get_next_proto(s);
			if (ret <= 0) goto end;
			s->init_num = 0;
			s->state=SSL3_ST_SR_FINISHED_A;
			break;
#endif
		case SSL3_ST_SR_FINISHED_A:
		case SSL3_ST_SR_FINISHED_B:
			if (!s->s3->change_cipher_spec)
				s->s3->flags |= SSL3_FLAGS_CCS_OK;
			ret=ssl3_get_finished(s,SSL3_ST_SR_FINISHED_A,
				SSL3_ST_SR_FINISHED_B);
			if (ret <= 0) goto end;
			if (s->hit)
				s->state=SSL_ST_OK;
#ifndef OPENSSL_NO_TLSEXT
			else if (s->tlsext_ticket_expected)
				s->state=SSL3_ST_SW_SESSION_TICKET_A;
#endif
			else
				s->state=SSL3_ST_SW_CHANGE_A;
			s->init_num=0;
			break;
#ifndef OPENSSL_NO_TLSEXT
		case SSL3_ST_SW_SESSION_TICKET_A:
		case SSL3_ST_SW_SESSION_TICKET_B:
			ret=ssl3_send_newsession_ticket(s);
			if (ret <= 0) goto end;
			s->state=SSL3_ST_SW_CHANGE_A;
			s->init_num=0;
			break;
		case SSL3_ST_SW_CERT_STATUS_A:
		case SSL3_ST_SW_CERT_STATUS_B:
			ret=ssl3_send_cert_status(s);
			if (ret <= 0) goto end;
			s->state=SSL3_ST_SW_KEY_EXCH_A;
			s->init_num=0;
			break;
#endif
		case SSL3_ST_SW_CHANGE_A:
		case SSL3_ST_SW_CHANGE_B:
			s->session->cipher=s->s3->tmp.new_cipher;
			if (!s->method->ssl3_enc->setup_key_block(s))
				{ ret= -1; goto end; }
			ret=ssl3_send_change_cipher_spec(s,
				SSL3_ST_SW_CHANGE_A,SSL3_ST_SW_CHANGE_B);
			if (ret <= 0) goto end;
			s->state=SSL3_ST_SW_FINISHED_A;
			s->init_num=0;
			if (!s->method->ssl3_enc->change_cipher_state(s,
				SSL3_CHANGE_CIPHER_SERVER_WRITE))
				{
				ret= -1;
				goto end;
				}
			break;
		case SSL3_ST_SW_FINISHED_A:
		case SSL3_ST_SW_FINISHED_B:
			ret=ssl3_send_finished(s,
				SSL3_ST_SW_FINISHED_A,SSL3_ST_SW_FINISHED_B,
				s->method->ssl3_enc->server_finished_label,
				s->method->ssl3_enc->server_finished_label_len);
			if (ret <= 0) goto end;
			s->state=SSL3_ST_SW_FLUSH;
			if (s->hit)
				{
#if defined(OPENSSL_NO_TLSEXT) || defined(OPENSSL_NO_NEXTPROTONEG)
				s->s3->tmp.next_state=SSL3_ST_SR_FINISHED_A;
#else
				if (s->s3->next_proto_neg_seen)
					{
					s->s3->tmp.next_state=SSL3_ST_SR_NEXT_PROTO_A;
					}
				else
					s->s3->tmp.next_state=SSL3_ST_SR_FINISHED_A;
#endif
				}
			else
				s->s3->tmp.next_state=SSL_ST_OK;
			s->init_num=0;
			break;
		case SSL_ST_OK:
			ssl3_cleanup_key_block(s);
			BUF_MEM_free(s->init_buf);
			s->init_buf=NULL;
			ssl_free_wbio_buffer(s);
			s->init_num=0;
			if (s->renegotiate == 2)  
				{
				s->renegotiate=0;
				s->new_session=0;
				ssl_update_cache(s,SSL_SESS_CACHE_SERVER);
				s->ctx->stats.sess_accept_good++;
				s->handshake_func=ssl3_accept;
				if (cb != NULL) cb(s,SSL_CB_HANDSHAKE_DONE,1);
				}
			ret = 1;
			goto end;
		default:
			SSLerr(SSL_F_SSL3_ACCEPT,SSL_R_UNKNOWN_STATE);
			ret= -1;
			goto end;
			}
		if (!s->s3->tmp.reuse_message && !skip)
			{
			if (s->debug)
				{
				if ((ret=BIO_flush(s->wbio)) <= 0)
					goto end;
				}
			if ((cb != NULL) && (s->state != state))
				{
				new_state=s->state;
				s->state=state;
				cb(s,SSL_CB_ACCEPT_LOOP,1);
				s->state=new_state;
				}
			}
		skip=0;
		}