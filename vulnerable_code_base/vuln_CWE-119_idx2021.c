dtls1_process_record(SSL *s)
{
	int i,al;
	int enc_err;
	SSL_SESSION *sess;
	SSL3_RECORD *rr;
	unsigned int mac_size;
	unsigned char md[EVP_MAX_MD_SIZE];
	rr= &(s->s3->rrec);
	sess = s->session;
	rr->input= &(s->packet[DTLS1_RT_HEADER_LENGTH]);
	if (rr->length > SSL3_RT_MAX_ENCRYPTED_LENGTH)
		{
		al=SSL_AD_RECORD_OVERFLOW;
		SSLerr(SSL_F_DTLS1_PROCESS_RECORD,SSL_R_ENCRYPTED_LENGTH_TOO_LONG);
		goto f_err;
		}
	rr->data=rr->input;
	rr->orig_len=rr->length;
	enc_err = s->method->ssl3_enc->enc(s,0);
	if (enc_err == 0)
		{
		rr->length = 0;
		s->packet_length = 0;
		goto err;
		}
#ifdef TLS_DEBUG
printf("dec %d\n",rr->length);
{ unsigned int z; for (z=0; z<rr->length; z++) printf("%02X%c",rr->data[z],((z+1)%16)?' ':'\n'); }
printf("\n");
#endif
	if ((sess != NULL) &&
	    (s->enc_read_ctx != NULL) &&
	    (EVP_MD_CTX_md(s->read_hash) != NULL))
		{
		unsigned char *mac = NULL;
		unsigned char mac_tmp[EVP_MAX_MD_SIZE];
		mac_size=EVP_MD_CTX_size(s->read_hash);
		OPENSSL_assert(mac_size <= EVP_MAX_MD_SIZE);
		if (rr->orig_len < mac_size ||
		    (EVP_CIPHER_CTX_mode(s->enc_read_ctx) == EVP_CIPH_CBC_MODE &&
		     rr->orig_len < mac_size+1))
			{
			al=SSL_AD_DECODE_ERROR;
			SSLerr(SSL_F_DTLS1_PROCESS_RECORD,SSL_R_LENGTH_TOO_SHORT);
			goto f_err;
			}
		if (EVP_CIPHER_CTX_mode(s->enc_read_ctx) == EVP_CIPH_CBC_MODE)
			{
			mac = mac_tmp;
			ssl3_cbc_copy_mac(mac_tmp, rr, mac_size);
			rr->length -= mac_size;
			}
		else
			{
			rr->length -= mac_size;
			mac = &rr->data[rr->length];
			}
		i=s->method->ssl3_enc->mac(s,md,0  );
		if (i < 0 || mac == NULL || CRYPTO_memcmp(md, mac, (size_t)mac_size) != 0)
			enc_err = -1;
		if (rr->length > SSL3_RT_MAX_COMPRESSED_LENGTH+mac_size)
			enc_err = -1;
		}
	if (enc_err < 0)
		{
		rr->length = 0;
		s->packet_length = 0;
		goto err;
		}
	if (s->expand != NULL)
		{
		if (rr->length > SSL3_RT_MAX_COMPRESSED_LENGTH)
			{
			al=SSL_AD_RECORD_OVERFLOW;
			SSLerr(SSL_F_DTLS1_PROCESS_RECORD,SSL_R_COMPRESSED_LENGTH_TOO_LONG);
			goto f_err;
			}
		if (!ssl3_do_uncompress(s))
			{
			al=SSL_AD_DECOMPRESSION_FAILURE;
			SSLerr(SSL_F_DTLS1_PROCESS_RECORD,SSL_R_BAD_DECOMPRESSION);
			goto f_err;
			}
		}
	if (rr->length > SSL3_RT_MAX_PLAIN_LENGTH)
		{
		al=SSL_AD_RECORD_OVERFLOW;
		SSLerr(SSL_F_DTLS1_PROCESS_RECORD,SSL_R_DATA_LENGTH_TOO_LONG);
		goto f_err;
		}
	rr->off=0;
	s->packet_length=0;
	dtls1_record_bitmap_update(s, &(s->d1->bitmap)); 
	return(1);
f_err:
	ssl3_send_alert(s,SSL3_AL_FATAL,al);
err:
	return(0);
}