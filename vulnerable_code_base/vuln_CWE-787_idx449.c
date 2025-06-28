wstran_pipe_recv_cb(void *arg)
{
	ws_pipe *p = arg;
	nni_iov  iov[2];
	uint8_t  rv;
	uint32_t pos = 1;
	uint64_t len = 0;
	uint8_t *ptr;
	nni_msg *smsg = NULL, *msg = NULL;
	nni_aio *raio = p->rxaio;
	nni_aio *uaio = NULL;
	bool     ack  = false;
	nni_mtx_lock(&p->mtx);
	if (p->user_rxaio != NULL) {
		uaio = p->user_rxaio;
	}
	if ((rv = nni_aio_result(raio)) != 0) {
		log_warn(" recv aio error %s", nng_strerror(rv));
		goto reset;
	}
	msg = nni_aio_get_msg(raio);
	if (nni_msg_header_len(msg) == 0 && nni_msg_len(msg) == 0) {
		log_trace("empty msg received! continue next receive");
		goto recv;
	}
	ptr = nni_msg_body(msg);
	p->gotrxhead += nni_msg_len(msg);
	log_trace("#### wstran_pipe_recv_cb got %ld msg: %p %x %ld",
	    p->gotrxhead, ptr, *ptr, nni_msg_len(msg));
	if (p->tmp_msg == NULL && p->gotrxhead > 0) {
		if ((rv = nni_msg_alloc(&p->tmp_msg, 0)) != 0) {
			log_error("mem error %ld\n", (size_t) len);
			goto reset;
		}
	}
	nni_msg_append(p->tmp_msg, ptr, nni_msg_len(msg));
	ptr = nni_msg_body(p->tmp_msg);  
	if (p->wantrxhead == 0) {
		if (p->gotrxhead == 1) {
			goto recv;
		}
		len = get_var_integer(ptr, &pos);
		if (*(ptr + pos - 1) > 0x7f) {
			if (p->gotrxhead >= NNI_NANO_MAX_HEADER_SIZE) {
				rv = NNG_EMSGSIZE;
				goto reset;
			}
		} else {
			p->wantrxhead = len + pos;
			nni_msg_set_cmd_type(p->tmp_msg, *ptr & 0xf0);
		}
	}
	if (p->gotrxhead >= p->wantrxhead) {
		goto done;
	}
recv:
	nni_msg_free(msg);
	nng_stream_recv(p->ws, raio);
	nni_mtx_unlock(&p->mtx);
	return;
done:
	if (uaio == NULL) {
		uaio = p->ep_aio;
	}
	if (uaio != NULL) {
		if (p->gotrxhead+p->wantrxhead > p->conf->max_packet_size) {
			log_trace("size error 0x95\n");
			rv = NMQ_PACKET_TOO_LARGE;
			goto recv_error;
		}
		p->gotrxhead  = 0;
		p->wantrxhead = 0;
		nni_msg_free(msg);
		if (nni_msg_cmd_type(p->tmp_msg) == CMD_CONNECT) {
			if (p->ws_param == NULL) {
				conn_param_alloc(&p->ws_param);
			}
			if (conn_handler(nni_msg_body(p->tmp_msg), p->ws_param,
			        nni_msg_len(p->tmp_msg)) != 0) {
				conn_param_free(p->ws_param);
				rv = NNG_ECONNRESET;
				goto reset;
			}
			if (p->ws_param->pro_ver == 5) {
				p->qsend_quota = p->ws_param->rx_max;
			}
			if (p->ws_param->max_packet_size == 0) {
				p->ws_param->max_packet_size =
				    p->conf->client_max_packet_size;
			}
			nni_msg_free(p->tmp_msg);
			p->tmp_msg = NULL;
			nni_aio_set_output(uaio, 0, p);
			nni_aio_finish(uaio, 0, 0);
			nni_mtx_unlock(&p->mtx);
			return;
		} else {
			if (nni_msg_alloc(&smsg, 0) != 0) {
				goto reset;
			}
			ws_msg_adaptor(ptr, smsg);
			nni_msg_free(p->tmp_msg);
			p->tmp_msg = NULL;
			nni_msg_set_conn_param(smsg, p->ws_param);
		}
		uint8_t   qos_pac;
		property *prop        = NULL;
		uint8_t   reason_code = 0;
		uint8_t   ack_cmd     = 0;
		uint16_t packet_id = 0;
		nni_msg *qmsg;
		uint8_t  cmd = nni_msg_cmd_type(smsg);
		if (cmd == CMD_PUBLISH) {
			qos_pac = nni_msg_get_pub_qos(smsg);
			if (qos_pac > 0) {
				if (p->ws_param->pro_ver == 5) {
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
				packet_id = nni_msg_get_pub_pid(smsg);
				ack       = true;
			}
		} else if (cmd == CMD_PUBREC) {
			if (nni_mqtt_pubres_decode(smsg, &packet_id, &reason_code, &prop,
			        p->ws_param->pro_ver) != 0) {
				log_trace("decode PUBREC variable header failed!");
			}
			ack_cmd = CMD_PUBREL;
			ack     = true;
		} else if (cmd == CMD_PUBREL) {
			if (nni_mqtt_pubres_decode(smsg, &packet_id, &reason_code, &prop,
			        p->ws_param->pro_ver) != 0) {
				log_trace("decode PUBREL variable header failed!");
			}
			ack_cmd = CMD_PUBCOMP;
			ack     = true;
		} else if (cmd == CMD_PUBACK || cmd == CMD_PUBCOMP) {
			if (nni_mqtt_pubres_decode(smsg, &packet_id, &reason_code, &prop,
			        p->ws_param->pro_ver) != 0) {
				log_trace("decode PUBACK or PUBCOMP variable header "
				          "failed!");
			}
			if (p->ws_param->pro_ver == 5) {
				property_free(prop);
				p->qsend_quota++;
			}
		} else if (cmd == CMD_PINGREQ) {
			ack = true;
		}
		if (ack == true) {
			if ((rv = nni_msg_alloc(&qmsg, 0)) != 0) {
				ack = false;
				rv  = NMQ_SERVER_BUSY;
				log_error("ERROR: OOM in WebSocket");
				goto recv_error;
			}
			if (cmd == CMD_PINGREQ) {
				uint8_t buf[2] = { CMD_PINGRESP, 0x00 };
				nni_msg_set_cmd_type(qmsg, CMD_PINGRESP);
				nni_msg_header_append(qmsg, buf, 2);
				nng_aio_wait(p->qsaio);
				iov[0].iov_len = nni_msg_header_len(qmsg);
				iov[0].iov_buf = nni_msg_header(qmsg);
				nni_aio_set_msg(p->qsaio, qmsg);
				nni_aio_set_iov(p->qsaio, 1, iov);
				nng_stream_send(p->ws, p->qsaio);
			} else {
				nni_msg_set_cmd_type(qmsg, ack_cmd);
				nni_mqtt_msgack_encode(qmsg, packet_id, reason_code,
				    prop, p->ws_param->pro_ver);
				nni_mqtt_pubres_header_encode(qmsg, ack_cmd);
				nng_aio_wait(p->qsaio);
				iov[0].iov_len = nni_msg_header_len(qmsg);
				iov[0].iov_buf = nni_msg_header(qmsg);
				iov[1].iov_len = nni_msg_len(qmsg);
				iov[1].iov_buf = nni_msg_body(qmsg);
				nni_aio_set_msg(p->qsaio, qmsg);
				nni_aio_set_iov(p->qsaio, 2, iov);
				nng_stream_send(p->ws, p->qsaio);
			}
		}
		nni_aio_set_msg(uaio, smsg);
		nni_aio_set_output(uaio, 0, p);
	} else {
		goto reset;
	}
	nni_mtx_unlock(&p->mtx);
	nni_aio_finish(uaio, 0, nni_msg_len(smsg));
	return;
reset:
	p->gotrxhead  = 0;
	p->wantrxhead = 0;
	nng_stream_close(p->ws);
	if (uaio != NULL) {
		nni_aio_finish_error(uaio, rv);
	} else if (p->ep_aio != NULL) {
		nni_aio_finish_error(p->ep_aio, rv);
	}
	if (p->tmp_msg != NULL) {
		smsg = p->tmp_msg;
		nni_msg_free(smsg);
		p->tmp_msg = NULL;
	}
	nni_mtx_unlock(&p->mtx);
	return;
recv_error:
	nni_pipe_bump_error(p->npipe, rv);
	nni_mtx_unlock(&p->mtx);
	nni_msg_free(msg);
	log_error("tcptran_pipe_recv_cb: recv error rv: %d\n", rv);
	return;
}