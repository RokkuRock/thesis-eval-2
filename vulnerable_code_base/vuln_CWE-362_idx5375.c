static pj_bool_t ssock_on_accept_complete (pj_ssl_sock_t *ssock_parent,
					   pj_sock_t newsock,
					   void *newconn,
					   const pj_sockaddr_t *src_addr,
					   int src_addr_len,
					   pj_status_t accept_status)
{
    pj_ssl_sock_t *ssock;
#ifndef SSL_SOCK_IMP_USE_OWN_NETWORK
    pj_activesock_cb asock_cb;
#endif
    pj_activesock_cfg asock_cfg;
    unsigned i;
    pj_status_t status;
#ifndef SSL_SOCK_IMP_USE_OWN_NETWORK
    PJ_UNUSED_ARG(newconn);
#endif
    if (accept_status != PJ_SUCCESS) {
	if (ssock_parent->param.cb.on_accept_complete2) {
	    (*ssock_parent->param.cb.on_accept_complete2)(ssock_parent, NULL,
						    	  src_addr,
						    	  src_addr_len,
						    	  accept_status);
	}
	return PJ_TRUE;
    }
    status = pj_ssl_sock_create(ssock_parent->pool,
				&ssock_parent->newsock_param, &ssock);
    if (status != PJ_SUCCESS)
	goto on_return;
    ssock->sock = newsock;
    ssock->parent = ssock_parent;
    ssock->is_server = PJ_TRUE;
    if (ssock_parent->cert) {
	status = pj_ssl_sock_set_certificate(ssock, ssock->pool, 
					     ssock_parent->cert);
	if (status != PJ_SUCCESS)
	    goto on_return;
    }
    ssock->addr_len = src_addr_len;
    pj_sockaddr_cp(&ssock->local_addr, &ssock_parent->local_addr);
    pj_sockaddr_cp(&ssock->rem_addr, src_addr);
    status = ssl_create(ssock);
    if (status != PJ_SUCCESS)
	goto on_return;
    ssock->asock_rbuf = (void**)pj_pool_calloc(ssock->pool, 
					       ssock->param.async_cnt,
					       sizeof(void*));
    if (!ssock->asock_rbuf)
        return PJ_ENOMEM;
    for (i = 0; i<ssock->param.async_cnt; ++i) {
	ssock->asock_rbuf[i] = (void*) pj_pool_alloc(
					    ssock->pool, 
					    ssock->param.read_buffer_size + 
					    sizeof(read_data_t*));
        if (!ssock->asock_rbuf[i])
            return PJ_ENOMEM;
    }
    if (ssock_parent->param.grp_lock) {
	pj_grp_lock_t *glock;
	status = pj_grp_lock_create(ssock->pool, NULL, &glock);
	if (status != PJ_SUCCESS)
	    goto on_return;
	pj_grp_lock_add_ref(glock);
	asock_cfg.grp_lock = ssock->param.grp_lock = glock;
	pj_grp_lock_add_handler(ssock->param.grp_lock, ssock->pool, ssock,
				ssl_on_destroy);
    }
#ifdef SSL_SOCK_IMP_USE_OWN_NETWORK
    status = network_setup_connection(ssock, newconn);
    if (status != PJ_SUCCESS)
	goto on_return;
#else
    status = pj_sock_apply_qos2(ssock->sock, ssock->param.qos_type,
				&ssock->param.qos_params, 1, 
				ssock->pool->obj_name, NULL);
    if (status != PJ_SUCCESS && !ssock->param.qos_ignore_error)
	goto on_return;
    if (ssock->param.sockopt_params.cnt) {
	status = pj_sock_setsockopt_params(ssock->sock, 
					   &ssock->param.sockopt_params);
	if (status != PJ_SUCCESS && !ssock->param.sockopt_ignore_error)
	    goto on_return;
    }
    pj_activesock_cfg_default(&asock_cfg);
    asock_cfg.async_cnt = ssock->param.async_cnt;
    asock_cfg.concurrency = ssock->param.concurrency;
    asock_cfg.whole_data = PJ_TRUE;
    pj_bzero(&asock_cb, sizeof(asock_cb));
    asock_cb.on_data_read = asock_on_data_read;
    asock_cb.on_data_sent = asock_on_data_sent;
    status = pj_activesock_create(ssock->pool,
				  ssock->sock, 
				  ssock->param.sock_type,
				  &asock_cfg,
				  ssock->param.ioqueue, 
				  &asock_cb,
				  ssock,
				  &ssock->asock);
    if (status != PJ_SUCCESS)
	goto on_return;
    status = pj_activesock_start_read2(ssock->asock, ssock->pool, 
				       (unsigned)ssock->param.read_buffer_size,
				       ssock->asock_rbuf,
				       PJ_IOQUEUE_ALWAYS_ASYNC);
    if (status != PJ_SUCCESS)
	goto on_return;
#endif
    status = get_localaddr(ssock, &ssock->local_addr, &ssock->addr_len);
    if (status != PJ_SUCCESS) {
	pj_sockaddr_cp(&ssock->local_addr, &ssock_parent->local_addr);
    }
    pj_assert(ssock->send_buf.max_len == 0);
    ssock->send_buf.buf = (char*)
			  pj_pool_alloc(ssock->pool, 
					ssock->param.send_buffer_size);
    if (!ssock->send_buf.buf)
        return PJ_ENOMEM;
    ssock->send_buf.max_len = ssock->param.send_buffer_size;
    ssock->send_buf.start = ssock->send_buf.buf;
    ssock->send_buf.len = 0;
    if (ssock->param.timer_heap && (ssock->param.timeout.sec != 0 ||
	ssock->param.timeout.msec != 0))
    {
	pj_assert(ssock->timer.id == TIMER_NONE);
	status = pj_timer_heap_schedule_w_grp_lock(ssock->param.timer_heap, 
						   &ssock->timer,
						   &ssock->param.timeout,
						   TIMER_HANDSHAKE_TIMEOUT,
						   ssock->param.grp_lock);
	if (status != PJ_SUCCESS) {
	    ssock->timer.id = TIMER_NONE;
	    status = PJ_SUCCESS;
	}
    }
    ssock->ssl_state = SSL_STATE_HANDSHAKING;
    ssl_set_state(ssock, PJ_TRUE);
    status = ssl_do_handshake(ssock);
on_return:
    if (ssock && status != PJ_EPENDING) {
	on_handshake_complete(ssock, status);
    }
    return PJ_TRUE;
}