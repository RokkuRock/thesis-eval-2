int dtls1_get_record(SSL *s)
	{
	int ssl_major,ssl_minor;
	int i,n;
	SSL3_RECORD *rr;
	unsigned char *p = NULL;
	unsigned short version;
	DTLS1_BITMAP *bitmap;
	unsigned int is_next_epoch;
	rr= &(s->s3->rrec);
	dtls1_process_buffered_records(s);
	if (dtls1_get_processed_record(s))
		return 1;
again:
	if (	(s->rstate != SSL_ST_READ_BODY) ||
		(s->packet_length < DTLS1_RT_HEADER_LENGTH)) 
		{
		n=ssl3_read_n(s, DTLS1_RT_HEADER_LENGTH, s->s3->rbuf.len, 0);
		if (n <= 0) return(n);  
		if (s->packet_length != DTLS1_RT_HEADER_LENGTH)
			{
			s->packet_length = 0;
			goto again;
			}
		s->rstate=SSL_ST_READ_BODY;
		p=s->packet;
		if (s->msg_callback)
			s->msg_callback(0, 0, SSL3_RT_HEADER, p, DTLS1_RT_HEADER_LENGTH, s, s->msg_callback_arg);
		rr->type= *(p++);
		ssl_major= *(p++);
		ssl_minor= *(p++);
		version=(ssl_major<<8)|ssl_minor;
		n2s(p,rr->epoch);
		memcpy(&(s->s3->read_sequence[2]), p, 6);
		p+=6;
		n2s(p,rr->length);
		if (!s->first_packet)
			{
			if (version != s->version)
				{
				rr->length = 0;
				s->packet_length = 0;
				goto again;
				}
			}
		if ((version & 0xff00) != (s->version & 0xff00))
			{
			rr->length = 0;
			s->packet_length = 0;
			goto again;
			}
		if (rr->length > SSL3_RT_MAX_ENCRYPTED_LENGTH)
			{
			rr->length = 0;
			s->packet_length = 0;
			goto again;
			}
		}
	if (rr->length > s->packet_length-DTLS1_RT_HEADER_LENGTH)
		{
		i=rr->length;
		n=ssl3_read_n(s,i,i,1);
		if ( n != i)
			{
			rr->length = 0;
			s->packet_length = 0;
			goto again;
			}
		}
	s->rstate=SSL_ST_READ_HEADER;  
	bitmap = dtls1_get_bitmap(s, rr, &is_next_epoch);
	if ( bitmap == NULL)
		{
		rr->length = 0;
		s->packet_length = 0;   
		goto again;    
		}
#ifndef OPENSSL_NO_SCTP
	if (!BIO_dgram_is_sctp(SSL_get_rbio(s)))
  		{
#endif
		if (!(s->d1->listen && rr->type == SSL3_RT_HANDSHAKE &&
		    s->packet_length > DTLS1_RT_HEADER_LENGTH &&
		    s->packet[DTLS1_RT_HEADER_LENGTH] == SSL3_MT_CLIENT_HELLO) &&
		    !dtls1_record_replay_check(s, bitmap))
			{
			rr->length = 0;
			s->packet_length=0;  
			goto again;      
			}
#ifndef OPENSSL_NO_SCTP
  		}
#endif
	if (rr->length == 0) goto again;
	if (is_next_epoch)
		{
		if ((SSL_in_init(s) || s->in_handshake) && !s->d1->listen)
			{
			dtls1_buffer_record(s, &(s->d1->unprocessed_rcds), rr->seq_num);
			}
		rr->length = 0;
		s->packet_length = 0;
		goto again;
		}
	if (!dtls1_process_record(s))
		{
		rr->length = 0;
		s->packet_length = 0;   
		goto again;    
		}
	return(1);
	}