static int tls_new_ciphertext ( struct tls_connection *tls,
				struct tls_header *tlshdr,
				struct list_head *rx_data ) {
	struct tls_cipherspec *cipherspec = &tls->rx_cipherspec;
	struct tls_cipher_suite *suite = cipherspec->suite;
	struct cipher_algorithm *cipher = suite->cipher;
	struct digest_algorithm *digest = suite->digest;
	size_t len = ntohs ( tlshdr->length );
	struct {
		uint8_t fixed[suite->fixed_iv_len];
		uint8_t record[suite->record_iv_len];
	} __attribute__ (( packed )) iv;
	struct tls_auth_header authhdr;
	uint8_t verify_mac[digest->digestsize];
	struct io_buffer *first;
	struct io_buffer *last;
	struct io_buffer *iobuf;
	void *mac;
	size_t check_len;
	int pad_len;
	int rc;
	assert ( ! list_empty ( rx_data ) );
	first = list_first_entry ( rx_data, struct io_buffer, list );
	last = list_last_entry ( rx_data, struct io_buffer, list );
	if ( iob_len ( first ) < sizeof ( iv.record ) ) {
		DBGC ( tls, "TLS %p received underlength IV\n", tls );
		DBGC_HD ( tls, first->data, iob_len ( first ) );
		return -EINVAL_IV;
	}
	memcpy ( iv.fixed, cipherspec->fixed_iv, sizeof ( iv.fixed ) );
	memcpy ( iv.record, first->data, sizeof ( iv.record ) );
	iob_pull ( first, sizeof ( iv.record ) );
	len -= sizeof ( iv.record );
	authhdr.seq = cpu_to_be64 ( tls->rx_seq );
	authhdr.header.type = tlshdr->type;
	authhdr.header.version = tlshdr->version;
	authhdr.header.length = htons ( len );
	cipher_setiv ( cipher, cipherspec->cipher_ctx, &iv, sizeof ( iv ) );
	check_len = 0;
	list_for_each_entry ( iobuf, &tls->rx_data, list ) {
		cipher_decrypt ( cipher, cipherspec->cipher_ctx,
				 iobuf->data, iobuf->data, iob_len ( iobuf ) );
		check_len += iob_len ( iobuf );
	}
	assert ( check_len == len );
	if ( is_block_cipher ( cipher ) ) {
		pad_len = tls_verify_padding ( tls, last );
		if ( pad_len < 0 ) {
			rc = pad_len;
			return rc;
		}
		iob_unput ( last, pad_len );
		len -= pad_len;
	}
	if ( iob_len ( last ) < suite->mac_len ) {
		DBGC ( tls, "TLS %p received underlength MAC\n", tls );
		DBGC_HD ( tls, last->data, iob_len ( last ) );
		return -EINVAL_MAC;
	}
	iob_unput ( last, suite->mac_len );
	len -= suite->mac_len;
	mac = last->tail;
	DBGC2 ( tls, "Received plaintext data:\n" );
	check_len = 0;
	list_for_each_entry ( iobuf, rx_data, list ) {
		DBGC2_HD ( tls, iobuf->data, iob_len ( iobuf ) );
		check_len += iob_len ( iobuf );
	}
	assert ( check_len == len );
	authhdr.header.length = htons ( len );
	if ( suite->mac_len )
		tls_hmac_list ( cipherspec, &authhdr, rx_data, verify_mac );
	if ( memcmp ( mac, verify_mac, suite->mac_len ) != 0 ) {
		DBGC ( tls, "TLS %p failed MAC verification\n", tls );
		return -EINVAL_MAC;
	}
	if ( ( rc = tls_new_record ( tls, tlshdr->type, rx_data ) ) != 0 )
		return rc;
	return 0;
}