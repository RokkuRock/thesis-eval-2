int db__message_store_find(struct mosquitto *context, uint16_t mid, struct mosquitto_msg_store **stored)
{
	struct mosquitto_client_msg *tail;
	if(!context) return MOSQ_ERR_INVAL;
	*stored = NULL;
	DL_FOREACH(context->msgs_in.inflight, tail){
		if(tail->store->source_mid == mid){
			*stored = tail->store;
			return MOSQ_ERR_SUCCESS;
		}
	}
	DL_FOREACH(context->msgs_in.queued, tail){
		if(tail->store->source_mid == mid){
			*stored = tail->store;
			return MOSQ_ERR_SUCCESS;
		}
	}
	return 1;
}