struct mosquitto *context__init(mosq_sock_t sock)
{
	struct mosquitto *context;
	char address[1024];
	context = mosquitto__calloc(1, sizeof(struct mosquitto));
	if(!context) return NULL;
#ifdef WITH_EPOLL
	context->ident = id_client;
#else
	context->pollfd_index = -1;
#endif
	mosquitto__set_state(context, mosq_cs_new);
	context->sock = sock;
	context->last_msg_in = db.now_s;
	context->next_msg_out = db.now_s + 60;
	context->keepalive = 60;  
	context->clean_start = true;
	context->id = NULL;
	context->last_mid = 0;
	context->will = NULL;
	context->username = NULL;
	context->password = NULL;
	context->listener = NULL;
	context->acl_list = NULL;
	context->retain_available = true;
	context->is_bridge = false;
	context->in_packet.payload = NULL;
	packet__cleanup(&context->in_packet);
	context->out_packet = NULL;
	context->current_out_packet = NULL;
	context->out_packet_count = 0;
	context->address = NULL;
	if((int)sock >= 0){
		if(!net__socket_get_address(sock, address, 1024, &context->remote_port)){
			context->address = mosquitto__strdup(address);
		}
		if(!context->address){
			mosquitto__free(context);
			return NULL;
		}
	}
	context->bridge = NULL;
	context->msgs_in.inflight_maximum = db.config->max_inflight_messages;
	context->msgs_out.inflight_maximum = db.config->max_inflight_messages;
	context->msgs_in.inflight_quota = db.config->max_inflight_messages;
	context->msgs_out.inflight_quota = db.config->max_inflight_messages;
	context->max_qos = 2;
#ifdef WITH_TLS
	context->ssl = NULL;
#endif
	if((int)context->sock >= 0){
		HASH_ADD(hh_sock, db.contexts_by_sock, sock, sizeof(context->sock), context);
	}
	return context;
}