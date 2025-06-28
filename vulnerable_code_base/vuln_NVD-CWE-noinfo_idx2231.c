static void srtp_rtp_cb(pjmedia_tp_cb_param *param)
{
    transport_srtp *srtp = (transport_srtp *) param->user_data;
    void *pkt = param->pkt;
    pj_ssize_t size = param->size;
    int len = (int)size;
    srtp_err_status_t err;
    void (*cb)(void*, void*, pj_ssize_t) = NULL;
    void (*cb2)(pjmedia_tp_cb_param*) = NULL;
    void *cb_data = NULL;
    if (srtp->bypass_srtp) {
        if (srtp->rtp_cb2) {
            pjmedia_tp_cb_param param2 = *param;
            param2.user_data = srtp->user_data;
            srtp->rtp_cb2(&param2);
            param->rem_switch = param2.rem_switch;
        } else if (srtp->rtp_cb) {
	    srtp->rtp_cb(srtp->user_data, pkt, size);
	}
	return;
    }
    if (size < 0) {
	return;
    }
    {
	unsigned i;
	pj_status_t status;
	for (i=0; i < srtp->keying_cnt; i++) {
	    if (!srtp->keying[i]->op->send_rtp)
		continue;
	    status = pjmedia_transport_send_rtp(srtp->keying[i], pkt, size);
	    if (status != PJ_EIGNORED) {
		return;
	    }
	}
    }
    PJ_ASSERT_ON_FAIL( (((pj_ssize_t)pkt) & 0x03)==0, return );
    if (srtp->probation_cnt > 0)
	--srtp->probation_cnt;
    pj_lock_acquire(srtp->mutex);
    if (!srtp->session_inited) {
	pj_lock_release(srtp->mutex);
	return;
    }
    if (srtp->use_rtcp_mux) {
    	pjmedia_rtp_hdr *hdr = (pjmedia_rtp_hdr *)pkt;
	if (hdr->pt >= 64 && hdr->pt <= 95) {   
	    pj_lock_release(srtp->mutex);
	    srtp_rtcp_cb(srtp, pkt, size);
    	    return;
    	}
    }
#if TEST_ROC
    if (srtp->setting.rx_roc.ssrc == 0) {
	srtp_err_status_t status;
	srtp->rx_ssrc = ntohl(((pjmedia_rtp_hdr*)pkt)->ssrc);
    	status = srtp_set_stream_roc(srtp->srtp_rx_ctx, srtp->rx_ssrc, 
    			    	     (srtp->offerer_side? 2: 1));
	if (status == srtp_err_status_ok) {    	
    	    srtp->setting.rx_roc.ssrc = srtp->rx_ssrc;
	    srtp->setting.rx_roc.roc = (srtp->offerer_side? 2: 1);
	    PJ_LOG(4, (THIS_FILE, "Setting RX ROC from SSRC %d to %d",
		   		  srtp->rx_ssrc, srtp->setting.rx_roc.roc));
	} else {
	    PJ_LOG(4, (THIS_FILE, "Setting RX ROC %s",
	    	       get_libsrtp_errstr(status)));
	}
    }
#endif
    err = srtp_unprotect(srtp->srtp_rx_ctx, (pj_uint8_t*)pkt, &len);
#if PJMEDIA_SRTP_CHECK_RTP_SEQ_ON_RESTART
    if (srtp->probation_cnt > 0 &&
	(err == srtp_err_status_replay_old ||
	 err == srtp_err_status_replay_fail))
    {
	pjmedia_srtp_crypto tx, rx;
	pj_status_t status;
	pjmedia_transport_srtp_stop((pjmedia_transport*)srtp);
	tx = srtp->tx_policy;
	rx = srtp->rx_policy;
	status = pjmedia_transport_srtp_start((pjmedia_transport*)srtp,
					      &tx, &rx);
	if (status != PJ_SUCCESS) {
	    PJ_LOG(5,(srtp->pool->obj_name, "Failed to restart SRTP, err=%s",
		      get_libsrtp_errstr(err)));
	} else if (!srtp->bypass_srtp) {
	    err = srtp_unprotect(srtp->srtp_rx_ctx, (pj_uint8_t*)pkt, &len);
	}
    }
#if PJMEDIA_SRTP_CHECK_ROC_ON_RESTART
    else
#endif
#endif
#if PJMEDIA_SRTP_CHECK_ROC_ON_RESTART
    if (srtp->probation_cnt > 0 && err == srtp_err_status_auth_fail &&
	srtp->setting.prev_rx_roc.ssrc != 0 &&
	srtp->setting.prev_rx_roc.ssrc == srtp->setting.rx_roc.ssrc &&
	srtp->setting.prev_rx_roc.roc != srtp->setting.rx_roc.roc)
    {
        unsigned roc, new_roc;
	srtp_err_status_t status;
    	srtp_get_stream_roc(srtp->srtp_rx_ctx, srtp->setting.rx_roc.ssrc,
    			    &roc);
    	new_roc = (roc == srtp->setting.rx_roc.roc?
    		   srtp->setting.prev_rx_roc.roc: srtp->setting.rx_roc.roc);
    	status = srtp_set_stream_roc(srtp->srtp_rx_ctx,
    				     srtp->setting.rx_roc.ssrc, new_roc);
	if (status == srtp_err_status_ok) {
	    PJ_LOG(4, (srtp->pool->obj_name,
		       "Retrying to unprotect SRTP from ROC %d to new ROC %d",
		       roc, new_roc));
    	    err = srtp_unprotect(srtp->srtp_rx_ctx, (pj_uint8_t*)pkt, &len);
    	}
    }
#endif
    if (err != srtp_err_status_ok) {
	PJ_LOG(5,(srtp->pool->obj_name,
		  "Failed to unprotect SRTP, pkt size=%d, err=%s",
		  size, get_libsrtp_errstr(err)));
    } else {
	cb = srtp->rtp_cb;
	cb2 = srtp->rtp_cb2;
	cb_data = srtp->user_data;
	srtp->rx_ssrc = ntohl(((pjmedia_rtp_hdr*)pkt)->ssrc);
    }
    pj_lock_release(srtp->mutex);
    if (cb2) {
        pjmedia_tp_cb_param param2 = *param;
        param2.user_data = cb_data;
        param2.pkt = pkt;
        param2.size = len;
        (*cb2)(&param2);
        param->rem_switch = param2.rem_switch;
    } else if (cb) {
	(*cb)(cb_data, pkt, len);
    }
}