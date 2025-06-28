void context__cleanup(struct mosquitto *context, bool force_free)
{
	struct mosquitto__packet *packet;
	if(!context) return;
	if(force_free){
		context->clean_start = true;
	}
#ifdef WITH_BRIDGE
	if(context->bridge){
		bridge__cleanup(context);
	}
#endif
	alias__free_all(context);
	mosquitto__free(context->auth_method);
	context->auth_method = NULL;
	mosquitto__free(context->username);
	context->username = NULL;
	mosquitto__free(context->password);
	context->password = NULL;
	net__socket_close(context);
	if(force_free){
		sub__clean_session(context);
	}
	db__messages_delete(context, force_free);
	mosquitto__free(context->address);
	context->address = NULL;
	context__send_will(context);
	if(context->id){
		context__remove_from_by_id(context);
		mosquitto__free(context->id);
		context->id = NULL;
	}
	packet__cleanup(&(context->in_packet));
	if(context->current_out_packet){
		packet__cleanup(context->current_out_packet);
		mosquitto__free(context->current_out_packet);
		context->current_out_packet = NULL;
	}
	while(context->out_packet){
		packet__cleanup(context->out_packet);
		packet = context->out_packet;
		context->out_packet = context->out_packet->next;
		mosquitto__free(packet);
	}
	context->out_packet_count = 0;
#if defined(WITH_BROKER) && defined(__GLIBC__) && defined(WITH_ADNS)
	if(context->adns){
		gai_cancel(context->adns);
		mosquitto__free((struct addrinfo *)context->adns->ar_request);
		mosquitto__free(context->adns);
	}
#endif
	if(force_free){
		mosquitto__free(context);
	}
}