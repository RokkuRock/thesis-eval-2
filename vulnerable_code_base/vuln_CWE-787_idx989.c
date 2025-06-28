mqtts_tcptran_pipe_recv_cb(void *arg)
{
	nni_aio *           aio;
	nni_iov             iov;
	uint8_t             type, pos, flags;
	uint32_t            len = 0, rv;
	size_t              n;
	nni_msg *           msg, *qmsg;
	mqtts_tcptran_pipe *p     = arg;
	nni_aio *           rxaio = p->rxaio;
	bool                ack   = false;
	nni_mtx_lock(&p->mtx);
	aio = nni_list_first(&p->recvq);
	if ((rv = nni_aio_result(rxaio)) != 0) {
		rv = SERVER_UNAVAILABLE;
		goto recv_error;
	}
	n = nni_aio_count(rxaio);
	p->gotrxhead += n;
	nni_aio_iov_advance(rxaio, n);
	if (nni_aio_iov_count(rxaio) > 0) {
		nng_stream_recv(p->conn, rxaio);
		nni_mtx_unlock(&p->mtx);
		return;
	}
	rv = mqtt_get_remaining_length(p->rxlen, p->gotrxhead, &len, &pos);
	p->wantrxhead = len + 1 + pos;
	if (p->gotrxhead <= 5 && p->rxlen[p->gotrxhead - 1] > 0x7f) {
		if (p->gotrxhead == NNI_NANO_MAX_HEADER_SIZE) {
			rv = PACKET_TOO_LARGE;
			goto recv_error;
		}
		iov.iov_buf = &p->rxlen[p->gotrxhead];
		iov.iov_len = 1;
		nni_aio_set_iov(rxaio, 1, &iov);
		nng_stream_recv(p->conn, rxaio);
		nni_mtx_unlock(&p->mtx);
		return;
	}
	if (NULL == p->rxmsg) {
		if ((len > p->rcvmax) && (p->rcvmax > 0)) {
			rv = PACKET_TOO_LARGE;
			goto recv_error;
		}
		if ((rv = nni_msg_alloc(&p->rxmsg, (size_t) len)) != 0) {
			rv = UNSPECIFIED_ERROR;
			goto recv_error;
		}
		nni_msg_set_remaining_len(p->rxmsg, len);
		if (len != 0) {
			iov.iov_buf = nni_msg_body(p->rxmsg);
			iov.iov_len = (size_t) len;
			nni_aio_set_iov(rxaio, 1, &iov);
			nng_stream_recv(p->conn, rxaio);
			nni_mtx_unlock(&p->mtx);
			return;
		}
	}
	nni_aio_list_remove(aio);
	nni_msg_header_append(p->rxmsg, p->rxlen, pos + 1);
	msg      = p->rxmsg;
	p->rxmsg = NULL;
	n        = nni_msg_len(msg);
	type     = p->rxlen[0] & 0xf0;
	flags    = p->rxlen[0] & 0x0f;
	uint8_t   qos_pac;
	uint16_t  packet_id   = 0;
	uint8_t   reason_code = 0;
	property *prop        = NULL;
	uint8_t   ack_cmd     = 0;
	switch (type) {
	case CMD_PUBLISH:
		qos_pac = nni_msg_get_pub_qos(msg);
		if (qos_pac > 0) {
			if (qos_pac == 1) {
				ack_cmd = CMD_PUBACK;
			} else if (qos_pac == 2) {
				ack_cmd = CMD_PUBREC;
			}
			packet_id = nni_msg_get_pub_pid(msg);
			ack = true;
		}
		break;
	case CMD_PUBREC:
		if (nni_mqtt_pubres_decode(msg, &packet_id, &reason_code, &prop,
		        p->proto) != 0) {
			rv = PROTOCOL_ERROR;
			goto recv_error;
		}
		ack_cmd = CMD_PUBREL;
		ack     = true;
		break;
	case CMD_PUBREL:
		if (flags == 0x02) {
			if (nni_mqtt_pubres_decode(msg, &packet_id, &reason_code,
			        &prop, p->proto) != 0) {
				rv = PROTOCOL_ERROR;
				goto recv_error;
			}
			ack_cmd = CMD_PUBCOMP;
			ack     = true;
			break;
		} else {
			rv = PROTOCOL_ERROR;
			goto recv_error;
		}
	case CMD_PUBACK:
	case CMD_PUBCOMP:
		if (nni_mqtt_pubres_decode(
		        msg, &packet_id, &reason_code, &prop, p->proto) != 0) {
			rv = PROTOCOL_ERROR;
			goto recv_error;
		}
		if (p->proto == MQTT_PROTOCOL_VERSION_v5) {
			p->sndmax++;
		}
		break;
	default:
		break;
	}
	if (ack == true) {
		if ((rv = nni_msg_alloc(&qmsg, 0)) != 0) {
			ack = false;
			rv  = UNSPECIFIED_ERROR;
			goto recv_error;
		}
		nni_mqtt_msgack_encode(
		    qmsg, packet_id, reason_code, prop, p->proto);
		nni_mqtt_pubres_header_encode(qmsg, ack_cmd);
		if (p->proto == MQTT_PROTOCOL_VERSION_v5) {
			property_free(prop);
		}
		if (p->busy == false) {
			nni_msg_insert(qmsg, nni_msg_header(qmsg),
			    nni_msg_header_len(qmsg));
			iov.iov_len    = nni_msg_len(qmsg);
			iov.iov_buf    = nni_msg_body(qmsg);
			p->busy        = true;
			nni_aio_set_msg(p->qsaio, qmsg);
			nni_aio_set_iov(p->qsaio, 1, &iov);
			nng_stream_send(p->conn, p->qsaio);
		} else {
			if (nni_lmq_full(&p->rslmq)) {
				if (nni_lmq_cap(&p->rslmq) <=
				    NNG_TRAN_MAX_LMQ_SIZE) {
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
	nni_pipe_bump_rx(p->npipe, n);
	if (!nni_list_empty(&p->recvq)) {
		mqtts_tcptran_pipe_recv_start(p);
	}
#ifdef NNG_HAVE_MQTT_BROKER
	nni_msg_set_conn_param(msg, p->cparam);
#endif
	nni_aio_set_msg(aio, msg);
	p->pingcnt = 0;
	nni_mtx_unlock(&p->mtx);
	nni_aio_finish_sync(aio, 0, n);
	return;
recv_error:
	nni_aio_list_remove(aio);
	msg      = p->rxmsg;
	p->rxmsg = NULL;
	nni_pipe_bump_error(p->npipe, rv);
	nni_mtx_unlock(&p->mtx);
	nni_msg_free(msg);
	nni_aio_finish_error(aio, rv);
}