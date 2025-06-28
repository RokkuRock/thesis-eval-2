int dtls1_read_bytes(SSL *s, int type, unsigned char *buf, int len, int peek)
	{
	int al,i,j,ret;
	unsigned int n;
	SSL3_RECORD *rr;
	void (*cb)(const SSL *ssl,int type2,int val)=NULL;
	if (s->s3->rbuf.buf == NULL)  
		if (!ssl3_setup_buffers(s))
			return(-1);
	if ((type && (type != SSL3_RT_APPLICATION_DATA) && 
		(type != SSL3_RT_HANDSHAKE)) ||
	    (peek && (type != SSL3_RT_APPLICATION_DATA)))
		{
		SSLerr(SSL_F_DTLS1_READ_BYTES, ERR_R_INTERNAL_ERROR);
		return -1;
		}
	if ( (ret = have_handshake_fragment(s, type, buf, len, peek)))
		return ret;
#ifndef OPENSSL_NO_SCTP
	if ((!s->in_handshake && SSL_in_init(s)) ||
	    (BIO_dgram_is_sctp(SSL_get_rbio(s)) &&
	     (s->state == DTLS1_SCTP_ST_SR_READ_SOCK || s->state == DTLS1_SCTP_ST_CR_READ_SOCK) &&
	     s->s3->in_read_app_data != 2))
#else
	if (!s->in_handshake && SSL_in_init(s))
#endif
		{
		i=s->handshake_func(s);
		if (i < 0) return(i);
		if (i == 0)
			{
			SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_SSL_HANDSHAKE_FAILURE);
			return(-1);
			}
		}
start:
	s->rwstate=SSL_NOTHING;
	rr = &(s->s3->rrec);
	if (s->state == SSL_ST_OK && rr->length == 0)
		{
		pitem *item;
		item = pqueue_pop(s->d1->buffered_app_data.q);
		if (item)
			{
#ifndef OPENSSL_NO_SCTP
			if (BIO_dgram_is_sctp(SSL_get_rbio(s)))
				{
				DTLS1_RECORD_DATA *rdata = (DTLS1_RECORD_DATA *) item->data;
				BIO_ctrl(SSL_get_rbio(s), BIO_CTRL_DGRAM_SCTP_SET_RCVINFO, sizeof(rdata->recordinfo), &rdata->recordinfo);
				}
#endif
			dtls1_copy_record(s, item);
			OPENSSL_free(item->data);
			pitem_free(item);
			}
		}
	if (dtls1_handle_timeout(s) > 0)
		goto start;
	if ((rr->length == 0) || (s->rstate == SSL_ST_READ_BODY))
		{
		ret=dtls1_get_record(s);
		if (ret <= 0) 
			{
			ret = dtls1_read_failed(s, ret);
			if (ret <= 0)  
				return(ret);
			else
				goto start;
			}
		}
	if (s->d1->listen && rr->type != SSL3_RT_HANDSHAKE)
		{
		rr->length = 0;
		goto start;
		}
	if (s->s3->change_cipher_spec  
		&& (rr->type != SSL3_RT_HANDSHAKE))
		{
		dtls1_buffer_record(s, &(s->d1->buffered_app_data), rr->seq_num);
		rr->length = 0;
		goto start;
		}
	if (s->shutdown & SSL_RECEIVED_SHUTDOWN)
		{
		rr->length=0;
		s->rwstate=SSL_NOTHING;
		return(0);
		}
	if (type == rr->type)  
		{
		if (SSL_in_init(s) && (type == SSL3_RT_APPLICATION_DATA) &&
			(s->enc_read_ctx == NULL))
			{
			al=SSL_AD_UNEXPECTED_MESSAGE;
			SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_APP_DATA_IN_HANDSHAKE);
			goto f_err;
			}
		if (len <= 0) return(len);
		if ((unsigned int)len > rr->length)
			n = rr->length;
		else
			n = (unsigned int)len;
		memcpy(buf,&(rr->data[rr->off]),n);
		if (!peek)
			{
			rr->length-=n;
			rr->off+=n;
			if (rr->length == 0)
				{
				s->rstate=SSL_ST_READ_HEADER;
				rr->off=0;
				}
			}
#ifndef OPENSSL_NO_SCTP
			if (BIO_dgram_is_sctp(SSL_get_rbio(s)) &&
			    rr->type == SSL3_RT_APPLICATION_DATA &&
			    (s->state == DTLS1_SCTP_ST_SR_READ_SOCK || s->state == DTLS1_SCTP_ST_CR_READ_SOCK))
				{
				s->rwstate=SSL_READING;
				BIO_clear_retry_flags(SSL_get_rbio(s));
				BIO_set_retry_read(SSL_get_rbio(s));
				}
			if (BIO_dgram_is_sctp(SSL_get_rbio(s)) &&
			    s->d1->shutdown_received && !BIO_dgram_sctp_msg_waiting(SSL_get_rbio(s)))
				{
				s->shutdown |= SSL_RECEIVED_SHUTDOWN;
				return(0);
				}
#endif			
		return(n);
		}
		{
		unsigned int k, dest_maxlen = 0;
		unsigned char *dest = NULL;
		unsigned int *dest_len = NULL;
		if (rr->type == SSL3_RT_HANDSHAKE)
			{
			dest_maxlen = sizeof s->d1->handshake_fragment;
			dest = s->d1->handshake_fragment;
			dest_len = &s->d1->handshake_fragment_len;
			}
		else if (rr->type == SSL3_RT_ALERT)
			{
			dest_maxlen = sizeof(s->d1->alert_fragment);
			dest = s->d1->alert_fragment;
			dest_len = &s->d1->alert_fragment_len;
			}
#ifndef OPENSSL_NO_HEARTBEATS
		else if (rr->type == TLS1_RT_HEARTBEAT)
			{
			dtls1_process_heartbeat(s);
			rr->length = 0;
			s->rwstate=SSL_READING;
			BIO_clear_retry_flags(SSL_get_rbio(s));
			BIO_set_retry_read(SSL_get_rbio(s));
			return(-1);
			}
#endif
		else if (rr->type != SSL3_RT_CHANGE_CIPHER_SPEC)
			{
			if (rr->type == SSL3_RT_APPLICATION_DATA)
				{
				BIO *bio;
				s->s3->in_read_app_data=2;
				bio=SSL_get_rbio(s);
				s->rwstate=SSL_READING;
				BIO_clear_retry_flags(bio);
				BIO_set_retry_read(bio);
				return(-1);
				}
			al=SSL_AD_UNEXPECTED_MESSAGE;
			SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_UNEXPECTED_RECORD);
			goto f_err;
			}
		if (dest_maxlen > 0)
			{
			if ( rr->length < dest_maxlen)
				{
#ifdef DTLS1_AD_MISSING_HANDSHAKE_MESSAGE
				FIX ME
#endif
				s->rstate=SSL_ST_READ_HEADER;
				rr->length = 0;
				goto start;
				}
			for ( k = 0; k < dest_maxlen; k++)
				{
				dest[k] = rr->data[rr->off++];
				rr->length--;
				}
			*dest_len = dest_maxlen;
			}
		}
	if ((!s->server) &&
		(s->d1->handshake_fragment_len >= DTLS1_HM_HEADER_LENGTH) &&
		(s->d1->handshake_fragment[0] == SSL3_MT_HELLO_REQUEST) &&
		(s->session != NULL) && (s->session->cipher != NULL))
		{
		s->d1->handshake_fragment_len = 0;
		if ((s->d1->handshake_fragment[1] != 0) ||
			(s->d1->handshake_fragment[2] != 0) ||
			(s->d1->handshake_fragment[3] != 0))
			{
			al=SSL_AD_DECODE_ERROR;
			SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_BAD_HELLO_REQUEST);
			goto err;
			}
		if (s->msg_callback)
			s->msg_callback(0, s->version, SSL3_RT_HANDSHAKE, 
				s->d1->handshake_fragment, 4, s, s->msg_callback_arg);
		if (SSL_is_init_finished(s) &&
			!(s->s3->flags & SSL3_FLAGS_NO_RENEGOTIATE_CIPHERS) &&
			!s->s3->renegotiate)
			{
			s->d1->handshake_read_seq++;
			s->new_session = 1;
			ssl3_renegotiate(s);
			if (ssl3_renegotiate_check(s))
				{
				i=s->handshake_func(s);
				if (i < 0) return(i);
				if (i == 0)
					{
					SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_SSL_HANDSHAKE_FAILURE);
					return(-1);
					}
				if (!(s->mode & SSL_MODE_AUTO_RETRY))
					{
					if (s->s3->rbuf.left == 0)  
						{
						BIO *bio;
						s->rwstate=SSL_READING;
						bio=SSL_get_rbio(s);
						BIO_clear_retry_flags(bio);
						BIO_set_retry_read(bio);
						return(-1);
						}
					}
				}
			}
		goto start;
		}
	if (s->d1->alert_fragment_len >= DTLS1_AL_HEADER_LENGTH)
		{
		int alert_level = s->d1->alert_fragment[0];
		int alert_descr = s->d1->alert_fragment[1];
		s->d1->alert_fragment_len = 0;
		if (s->msg_callback)
			s->msg_callback(0, s->version, SSL3_RT_ALERT, 
				s->d1->alert_fragment, 2, s, s->msg_callback_arg);
		if (s->info_callback != NULL)
			cb=s->info_callback;
		else if (s->ctx->info_callback != NULL)
			cb=s->ctx->info_callback;
		if (cb != NULL)
			{
			j = (alert_level << 8) | alert_descr;
			cb(s, SSL_CB_READ_ALERT, j);
			}
		if (alert_level == 1)  
			{
			s->s3->warn_alert = alert_descr;
			if (alert_descr == SSL_AD_CLOSE_NOTIFY)
				{
#ifndef OPENSSL_NO_SCTP
				if (BIO_dgram_is_sctp(SSL_get_rbio(s)) &&
					BIO_dgram_sctp_msg_waiting(SSL_get_rbio(s)))
					{
					s->d1->shutdown_received = 1;
					s->rwstate=SSL_READING;
					BIO_clear_retry_flags(SSL_get_rbio(s));
					BIO_set_retry_read(SSL_get_rbio(s));
					return -1;
					}
#endif
				s->shutdown |= SSL_RECEIVED_SHUTDOWN;
				return(0);
				}
#if 0
			if (alert_descr == DTLS1_AD_MISSING_HANDSHAKE_MESSAGE)
				{
				unsigned short seq;
				unsigned int frag_off;
				unsigned char *p = &(s->d1->alert_fragment[2]);
				n2s(p, seq);
				n2l3(p, frag_off);
				dtls1_retransmit_message(s,
										 dtls1_get_queue_priority(frag->msg_header.seq, 0),
										 frag_off, &found);
				if ( ! found  && SSL_in_init(s))
					{
					ssl3_send_alert(s,SSL3_AL_WARNING,
						DTLS1_AD_MISSING_HANDSHAKE_MESSAGE);
					}
				}
#endif
			}
		else if (alert_level == 2)  
			{
			char tmp[16];
			s->rwstate=SSL_NOTHING;
			s->s3->fatal_alert = alert_descr;
			SSLerr(SSL_F_DTLS1_READ_BYTES, SSL_AD_REASON_OFFSET + alert_descr);
			BIO_snprintf(tmp,sizeof tmp,"%d",alert_descr);
			ERR_add_error_data(2,"SSL alert number ",tmp);
			s->shutdown|=SSL_RECEIVED_SHUTDOWN;
			SSL_CTX_remove_session(s->ctx,s->session);
			return(0);
			}
		else
			{
			al=SSL_AD_ILLEGAL_PARAMETER;
			SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_UNKNOWN_ALERT_TYPE);
			goto f_err;
			}
		goto start;
		}
	if (s->shutdown & SSL_SENT_SHUTDOWN)  
		{
		s->rwstate=SSL_NOTHING;
		rr->length=0;
		return(0);
		}
	if (rr->type == SSL3_RT_CHANGE_CIPHER_SPEC)
		{
		struct ccs_header_st ccs_hdr;
		unsigned int ccs_hdr_len = DTLS1_CCS_HEADER_LENGTH;
		dtls1_get_ccs_header(rr->data, &ccs_hdr);
		if (s->version == DTLS1_BAD_VER)
			ccs_hdr_len = 3;
		if (	(rr->length != ccs_hdr_len) || 
			(rr->off != 0) || (rr->data[0] != SSL3_MT_CCS))
			{
			i=SSL_AD_ILLEGAL_PARAMETER;
			SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_BAD_CHANGE_CIPHER_SPEC);
			goto err;
			}
		rr->length=0;
		if (s->msg_callback)
			s->msg_callback(0, s->version, SSL3_RT_CHANGE_CIPHER_SPEC, 
				rr->data, 1, s, s->msg_callback_arg);
		if (!s->d1->change_cipher_spec_ok)
			{
			goto start;
			}
		s->d1->change_cipher_spec_ok = 0;
		s->s3->change_cipher_spec=1;
		if (!ssl3_do_change_cipher_spec(s))
			goto err;
		dtls1_reset_seq_numbers(s, SSL3_CC_READ);
		if (s->version == DTLS1_BAD_VER)
			s->d1->handshake_read_seq++;
#ifndef OPENSSL_NO_SCTP
		BIO_ctrl(SSL_get_wbio(s), BIO_CTRL_DGRAM_SCTP_AUTH_CCS_RCVD, 1, NULL);
#endif
		goto start;
		}
	if ((s->d1->handshake_fragment_len >= DTLS1_HM_HEADER_LENGTH) && 
		!s->in_handshake)
		{
		struct hm_header_st msg_hdr;
		dtls1_get_message_header(rr->data, &msg_hdr);
		if( rr->epoch != s->d1->r_epoch)
			{
			rr->length = 0;
			goto start;
			}
		if (msg_hdr.type == SSL3_MT_FINISHED)
			{
			if (dtls1_check_timeout_num(s) < 0)
				return -1;
			dtls1_retransmit_buffered_messages(s);
			rr->length = 0;
			goto start;
			}
		if (((s->state&SSL_ST_MASK) == SSL_ST_OK) &&
			!(s->s3->flags & SSL3_FLAGS_NO_RENEGOTIATE_CIPHERS))
			{
#if 0  
			s->state=SSL_ST_BEFORE|(s->server)
				?SSL_ST_ACCEPT
				:SSL_ST_CONNECT;
#else
			s->state = s->server ? SSL_ST_ACCEPT : SSL_ST_CONNECT;
#endif
			s->renegotiate=1;
			s->new_session=1;
			}
		i=s->handshake_func(s);
		if (i < 0) return(i);
		if (i == 0)
			{
			SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_SSL_HANDSHAKE_FAILURE);
			return(-1);
			}
		if (!(s->mode & SSL_MODE_AUTO_RETRY))
			{
			if (s->s3->rbuf.left == 0)  
				{
				BIO *bio;
				s->rwstate=SSL_READING;
				bio=SSL_get_rbio(s);
				BIO_clear_retry_flags(bio);
				BIO_set_retry_read(bio);
				return(-1);
				}
			}
		goto start;
		}
	switch (rr->type)
		{
	default:
#ifndef OPENSSL_NO_TLS
		if (s->version == TLS1_VERSION)
			{
			rr->length = 0;
			goto start;
			}
#endif
		al=SSL_AD_UNEXPECTED_MESSAGE;
		SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_UNEXPECTED_RECORD);
		goto f_err;
	case SSL3_RT_CHANGE_CIPHER_SPEC:
	case SSL3_RT_ALERT:
	case SSL3_RT_HANDSHAKE:
		al=SSL_AD_UNEXPECTED_MESSAGE;
		SSLerr(SSL_F_DTLS1_READ_BYTES,ERR_R_INTERNAL_ERROR);
		goto f_err;
	case SSL3_RT_APPLICATION_DATA:
		if (s->s3->in_read_app_data &&
			(s->s3->total_renegotiations != 0) &&
			((
				(s->state & SSL_ST_CONNECT) &&
				(s->state >= SSL3_ST_CW_CLNT_HELLO_A) &&
				(s->state <= SSL3_ST_CR_SRVR_HELLO_A)
				) || (
					(s->state & SSL_ST_ACCEPT) &&
					(s->state <= SSL3_ST_SW_HELLO_REQ_A) &&
					(s->state >= SSL3_ST_SR_CLNT_HELLO_A)
					)
				))
			{
			s->s3->in_read_app_data=2;
			return(-1);
			}
		else
			{
			al=SSL_AD_UNEXPECTED_MESSAGE;
			SSLerr(SSL_F_DTLS1_READ_BYTES,SSL_R_UNEXPECTED_RECORD);
			goto f_err;
			}
		}
f_err:
	ssl3_send_alert(s,SSL3_AL_FATAL,al);
err:
	return(-1);
	}