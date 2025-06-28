static pj_bool_t on_handshake_complete(pj_ssl_sock_t *ssock, 
				       pj_status_t status)
{
    if (ssock->timer.id == TIMER_HANDSHAKE_TIMEOUT) {
	pj_timer_heap_cancel(ssock->param.timer_heap, &ssock->timer);
	ssock->timer.id = TIMER_NONE;
    }
    if (status == PJ_SUCCESS)
	ssl_update_certs_info(ssock);
    if (ssock->is_server) {
	if (status != PJ_SUCCESS) {
	    char buf[PJ_INET6_ADDRSTRLEN+10];
	    PJ_PERROR(3,(ssock->pool->obj_name, status,
			 "Handshake failed in accepting %s",
			 pj_sockaddr_print(&ssock->rem_addr, buf,
					   sizeof(buf), 3)));
	    if (ssock->param.cb.on_accept_complete2) {
		(*ssock->param.cb.on_accept_complete2) 
		      (ssock->parent, ssock, (pj_sockaddr_t*)&ssock->rem_addr, 
		      pj_sockaddr_get_len((pj_sockaddr_t*)&ssock->rem_addr), 
		      status);
	    }
#if 1  
	    if (ssock->param.timer_heap) {
		pj_time_val interval = {0, PJ_SSL_SOCK_DELAYED_CLOSE_TIMEOUT};
		pj_status_t status1;
		ssock->ssl_state = SSL_STATE_NULL;
		ssl_close_sockets(ssock);
		if (ssock->timer.id != TIMER_NONE) {
		    pj_timer_heap_cancel(ssock->param.timer_heap,
					 &ssock->timer);
		}
		pj_time_val_normalize(&interval);
		status1 = pj_timer_heap_schedule_w_grp_lock(
						 ssock->param.timer_heap, 
						 &ssock->timer,
						 &interval,
						 TIMER_CLOSE,
						 ssock->param.grp_lock);
		if (status1 != PJ_SUCCESS) {
	    	    PJ_PERROR(3,(ssock->pool->obj_name, status,
				 "Failed to schedule a delayed close. "
				 "Race condition may occur."));
		    ssock->timer.id = TIMER_NONE;
		    pj_ssl_sock_close(ssock);
		}
	    } else {
		pj_ssl_sock_close(ssock);
	    }
#else
	    {
		pj_ssl_sock_close(ssock);
	    }
#endif
	    return PJ_FALSE;
	}
	if (ssock->param.cb.on_accept_complete2) {
	    pj_bool_t ret;
	    ret = (*ssock->param.cb.on_accept_complete2) 
		    (ssock->parent, ssock, (pj_sockaddr_t*)&ssock->rem_addr, 
		    pj_sockaddr_get_len((pj_sockaddr_t*)&ssock->rem_addr), 
		    status);
	    if (ret == PJ_FALSE)
		return PJ_FALSE;	
	} else if (ssock->param.cb.on_accept_complete) {
	    pj_bool_t ret;
	    ret = (*ssock->param.cb.on_accept_complete)
		      (ssock->parent, ssock, (pj_sockaddr_t*)&ssock->rem_addr,
		       pj_sockaddr_get_len((pj_sockaddr_t*)&ssock->rem_addr));
	    if (ret == PJ_FALSE)
		return PJ_FALSE;
	}
    }
    else {
	if (status != PJ_SUCCESS) {
	    ssl_reset_sock_state(ssock);
	}
	if (ssock->param.cb.on_connect_complete) {
	    pj_bool_t ret;
	    ret = (*ssock->param.cb.on_connect_complete)(ssock, status);
	    if (ret == PJ_FALSE)
		return PJ_FALSE;
	}
    }
    return PJ_TRUE;
}