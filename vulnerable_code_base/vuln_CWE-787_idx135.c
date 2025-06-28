tlstran_pipe_recv_cb(void *arg)
{
	nni_aio *     aio;
	nni_iov       iov[2];
	uint8_t       type;
	uint8_t       rv;
	uint32_t      pos = 1;
	uint64_t      len = 0;
	size_t        n;
	nni_msg      *msg, *qmsg;
	tlstran_pipe *p     = arg;
	nni_aio *     rxaio = p->rxaio;
	conn_param *  cparam;
	bool          ack   = false;
	log_trace("tlstran_pipe_recv_cb %p\n", p);
	nni_mtx_lock(&p->mtx);
	aio = nni_list_first(&p->recvq);
	if ((rv = nni_aio_result(rxaio)) != 0) {
		log_warn(" recv aio error %s", nng_strerror(rv));
		rv = NMQ_SERVER_BUSY;
		goto recv_error;
	}
	n = nni_aio_count(rxaio);
	p->gotrxhead += n;
	nni_aio_iov_advance(rxaio, n);
	len = get_var_integer(p->rxlen, &pos);
	log_trace("new %ld recevied %ld header %x %d pos: %d len : %d", n,
	    p->gotrxhead, p->rxlen[0], p->rxlen[1], pos, len);
	log_trace("still need byte count:%ld > 0\n", nni_aio_iov_count(rxaio));
	if (nni_aio_iov_count(rxaio) > 0) {
		log_trace("got: %x %x, %ld!!\n", p->rxlen[0], p->rxlen[1],
		    strlen((char *) p->rxlen));
		nng_stream_recv(p->conn, rxaio);
		nni_mtx_unlock(&p->mtx);
		return;
	} else if (p->gotrxhead <= NNI_NANO_MAX_HEADER_SIZE &&
	    p->rxlen[p->gotrxhead - 1] > 0x7f) {
		if (p->gotrxhead == NNI_NANO_MAX_HEADER_SIZE) {
			rv = NNG_EMSGSIZE;
			goto recv_error;
		}
		iov[0].iov_buf = &p->rxlen[p->gotrxhead];
		iov[0].iov_len = 1;
		nni_aio_set_iov(rxaio, 1, iov);
		nng_stream_recv(p->conn, rxaio);
		nni_mtx_unlock(&p->mtx);
		return;
	} else if (len == 0 && n == 2) {
		if ((p->rxlen[0] & 0XFF) == CMD_PINGREQ) {
			nng_aio_wait(p->rpaio);
			p->txlen[0] = CMD_PINGRESP;
			p->txlen[1] = 0x00;
			iov[0].iov_len = 2;
			iov[0].iov_buf = &p->txlen;
			nni_aio_set_iov(p->rpaio, 1, iov);
			nng_stream_send(p->conn, p->rpaio);
			goto notify;
		}
	}
	p->wantrxhead = len + p->gotrxhead;
	cparam        = p->tcp_cparam;
	if (p->rxmsg == NULL) {
		log_trace("pipe %p header got: %x %x %x %x %x, %ld!!\n", p,
		    p->rxlen[0], p->rxlen[1], p->rxlen[2], p->rxlen[3],
		    p->rxlen[4], p->wantrxhead);
		if (len > p->conf->max_packet_size) {
			log_error("size error 0x95\n");
			rv = NMQ_PACKET_TOO_LARGE;
			goto recv_error;
		}
		if ((rv = nni_msg_alloc(&p->rxmsg, (size_t) len)) != 0) {
			log_error("mem error %ld\n", (size_t) len);
			rv = NMQ_SERVER_UNAVAILABLE;
			goto recv_error;
		}
		if (len != 0) {
			iov[0].iov_buf = nni_msg_body(p->rxmsg);
			iov[0].iov_len = (size_t) len;
			nni_aio_set_iov(rxaio, 1, iov);
			nng_stream_recv(p->conn, rxaio);
			nni_mtx_unlock(&p->mtx);
			return;
		}
	}
	nni_aio_list_remove(aio);
	msg      = p->rxmsg;
	p->rxmsg = NULL;
	n        = nni_msg_len(msg);
	type     = p->rxlen[0] & 0xf0;
	fixed_header_adaptor(p->rxlen, msg);
	nni_msg_set_conn_param(msg, cparam);
	nni_msg_set_remaining_len(msg, len);
	nni_msg_set_cmd_type(msg, type);
	log_trace("remain_len %d cparam %p clientid %s username %s proto %d\n",
	    len, cparam, cparam->clientid.body, cparam->username.body,
	    cparam->pro_ver);
	log_trace("The type of msg is %x", type);
	uint16_t  packet_id   = 0;
	uint8_t   reason_code = 0;
	property *prop        = NULL;
	uint8_t   ack_cmd     = 0;
	if (type == CMD_PUBLISH) {
		nni_msg_set_timestamp(msg, nng_clock());
		uint8_t qos_pac = nni_msg_get_pub_qos(msg);
		if (qos_pac > 0) {
			if (p->tcp_cparam->pro_ver == 5) {
				if (p->qrecv_quota > 0) {
					p->qrecv_quota--;
				} else {
					rv = NMQ_RECEIVE_MAXIMUM_EXCEEDED;
					goto recv_error;
				}
			}
			if (qos_pac == 1) {
				ack_cmd = CMD_PUBACK;
			} else if (qos_pac == 2) {
				ack_cmd = CMD_PUBREC;
			}
			packet_id = nni_msg_get_pub_pid(msg);
			ack       = true;
		}
	} else if (type == CMD_PUBREC) {
		if (nni_mqtt_pubres_decode(msg, &packet_id, &reason_code, &prop,
		        cparam->pro_ver) != 0) {
			log_error("decode PUBREC variable header failed!");
		}
		ack_cmd = CMD_PUBREL;
		ack     = true;
	} else if (type == CMD_PUBREL) {
		if (nni_mqtt_pubres_decode(msg, &packet_id, &reason_code, &prop,
		        cparam->pro_ver) != 0) {
			log_error("decode PUBREL variable header failed!");
		}
		ack_cmd = CMD_PUBCOMP;
		ack     = true;
	} else if (type == CMD_PUBACK || type == CMD_PUBCOMP) {
		if (nni_mqtt_pubres_decode(msg, &packet_id, &reason_code, &prop,
		        cparam->pro_ver) != 0) {
			log_error("decode PUBACK or PUBCOMP variable header "
			          "failed!");
		}
		if (p->tcp_cparam->pro_ver == 5) {
			property_free(prop);
			p->qsend_quota++;
		}
	}
	if (ack == true) {
		if ((rv = nni_msg_alloc(&qmsg, 0)) != 0) {
			ack = false;
			rv  = NMQ_SERVER_BUSY;
			goto recv_error;
		}
		nni_msg_set_cmd_type(qmsg, ack_cmd);
		nni_mqtt_msgack_encode(
		    qmsg, packet_id, reason_code, prop, cparam->pro_ver);
		nni_mqtt_pubres_header_encode(qmsg, ack_cmd);
		if (p->busy == false) {
			if (nni_aio_begin(aio) != 0) {
				log_error("ACK aio error!!");
			}
			nni_msg_insert(qmsg, nni_msg_header(qmsg),
			    nni_msg_header_len(qmsg));
			iov[0].iov_len = nni_msg_len(qmsg);
			iov[0].iov_buf = nni_msg_body(qmsg);
			p->busy        = true;
			nni_aio_set_msg(p->qsaio, qmsg);
			nni_aio_set_iov(p->qsaio, 1, iov);
			nng_stream_send(p->conn, p->qsaio);
			log_trace("QoS ACK msg sent!");
		} else {
			if (nni_lmq_full(&p->rslmq)) {
				if (nni_lmq_cap(&p->rslmq) <=
				    NANO_MAX_QOS_PACKET) {
					if ((rv = nni_lmq_resize(&p->rslmq,
					         nni_lmq_cap(&p->rslmq) *
					             2)) == 0) {
						nni_lmq_put(&p->rslmq, qmsg);
					} else {
						nni_msg_free(qmsg);
					}
				} else {
					nni_msg *old;
					(void) nni_lmq_get(&p->rslmq, &old);
					nni_msg_free(old);
					nni_lmq_put(&p->rslmq, qmsg);
				}
			} else {
				nni_lmq_put(&p->rslmq, qmsg);
			}
		}
		ack = false;
	}
	if (!nni_list_empty(&p->recvq)) {
		tlstran_pipe_recv_start(p);
	}
	nni_pipe_bump_rx(p->npipe, n);
	nni_mtx_unlock(&p->mtx);
	nni_aio_set_msg(aio, msg);
	nni_aio_finish_sync(aio, 0, n);
	log_trace("end of tlstran_pipe_recv_cb: synch! %p\n", p);
	return;
recv_error:
	nni_aio_list_remove(aio);
	msg      = p->rxmsg;
	p->rxmsg = NULL;
	nni_pipe_bump_error(p->npipe, rv);
	nni_mtx_unlock(&p->mtx);
	nni_msg_free(msg);
	nni_aio_finish_error(aio, rv);
	log_trace("tlstran_pipe_recv_cb: recv error rv: %d\n", rv);
	return;
notify:
	nni_aio_list_remove(aio);
	nni_mtx_unlock(&p->mtx);
	nni_aio_set_msg(aio, NULL);
	nni_aio_finish(aio, 0, 0);
	return;
}