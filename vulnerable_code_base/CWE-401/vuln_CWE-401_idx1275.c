int db__message_insert(struct mosquitto *context, uint16_t mid, enum mosquitto_msg_direction dir, uint8_t qos, bool retain, struct mosquitto_msg_store *stored, mosquitto_property *properties, bool update)
{
	struct mosquitto_client_msg *msg;
	struct mosquitto_msg_data *msg_data;
	enum mosquitto_msg_state state = mosq_ms_invalid;
	int rc = 0;
	int i;
	char **dest_ids;
	assert(stored);
	if(!context) return MOSQ_ERR_INVAL;
	if(!context->id) return MOSQ_ERR_SUCCESS;  
	if(dir == mosq_md_out){
		msg_data = &context->msgs_out;
	}else{
		msg_data = &context->msgs_in;
	}
	if(context->protocol != mosq_p_mqtt5
			&& db.config->allow_duplicate_messages == false
			&& dir == mosq_md_out && retain == false && stored->dest_ids){
		for(i=0; i<stored->dest_id_count; i++){
			if(stored->dest_ids[i] && !strcmp(stored->dest_ids[i], context->id)){
				mosquitto_property_free_all(&properties);
				return MOSQ_ERR_SUCCESS;
			}
		}
	}
	if(context->sock == INVALID_SOCKET){
		if(qos == 0 && !db.config->queue_qos0_messages){
			if(!context->bridge){
				mosquitto_property_free_all(&properties);
				return 2;
			}else{
				if(context->bridge->start_type != bst_lazy){
					mosquitto_property_free_all(&properties);
					return 2;
				}
			}
		}
		if(context->bridge && context->bridge->clean_start_local == true){
			mosquitto_property_free_all(&properties);
			return 2;
		}
	}
	if(context->sock != INVALID_SOCKET){
		if(db__ready_for_flight(context, dir, qos)){
			if(dir == mosq_md_out){
				switch(qos){
					case 0:
						state = mosq_ms_publish_qos0;
						break;
					case 1:
						state = mosq_ms_publish_qos1;
						break;
					case 2:
						state = mosq_ms_publish_qos2;
						break;
				}
			}else{
				if(qos == 2){
					state = mosq_ms_wait_for_pubrel;
				}else{
					mosquitto_property_free_all(&properties);
					return 1;
				}
			}
		}else if(qos != 0 && db__ready_for_queue(context, qos, msg_data)){
			state = mosq_ms_queued;
			rc = 2;
		}else{
			if(context->is_dropping == false){
				context->is_dropping = true;
				log__printf(NULL, MOSQ_LOG_NOTICE,
						"Outgoing messages are being dropped for client %s.",
						context->id);
			}
			G_MSGS_DROPPED_INC();
			mosquitto_property_free_all(&properties);
			return 2;
		}
	}else{
		if (db__ready_for_queue(context, qos, msg_data)){
			state = mosq_ms_queued;
		}else{
			G_MSGS_DROPPED_INC();
			if(context->is_dropping == false){
				context->is_dropping = true;
				log__printf(NULL, MOSQ_LOG_NOTICE,
						"Outgoing messages are being dropped for client %s.",
						context->id);
			}
			mosquitto_property_free_all(&properties);
			return 2;
		}
	}
	assert(state != mosq_ms_invalid);
#ifdef WITH_PERSISTENCE
	if(state == mosq_ms_queued){
		db.persistence_changes++;
	}
#endif
	msg = mosquitto__malloc(sizeof(struct mosquitto_client_msg));
	if(!msg) return MOSQ_ERR_NOMEM;
	msg->prev = NULL;
	msg->next = NULL;
	msg->store = stored;
	db__msg_store_ref_inc(msg->store);
	msg->mid = mid;
	msg->timestamp = db.now_s;
	msg->direction = dir;
	msg->state = state;
	msg->dup = false;
	if(qos > context->max_qos){
		msg->qos = context->max_qos;
	}else{
		msg->qos = qos;
	}
	msg->retain = retain;
	msg->properties = properties;
	if(state == mosq_ms_queued){
		DL_APPEND(msg_data->queued, msg);
		db__msg_add_to_queued_stats(msg_data, msg);
	}else{
		DL_APPEND(msg_data->inflight, msg);
		db__msg_add_to_inflight_stats(msg_data, msg);
	}
	if(db.config->allow_duplicate_messages == false && dir == mosq_md_out && retain == false){
		dest_ids = mosquitto__realloc(stored->dest_ids, sizeof(char *)*(size_t)(stored->dest_id_count+1));
		if(dest_ids){
			stored->dest_ids = dest_ids;
			stored->dest_id_count++;
			stored->dest_ids[stored->dest_id_count-1] = mosquitto__strdup(context->id);
			if(!stored->dest_ids[stored->dest_id_count-1]){
				return MOSQ_ERR_NOMEM;
			}
		}else{
			return MOSQ_ERR_NOMEM;
		}
	}
#ifdef WITH_BRIDGE
	if(context->bridge && context->bridge->start_type == bst_lazy
			&& context->sock == INVALID_SOCKET
			&& context->msgs_out.inflight_count + context->msgs_out.queued_count >= context->bridge->threshold){
		context->bridge->lazy_reconnect = true;
	}
#endif
	if(dir == mosq_md_out && msg->qos > 0 && state != mosq_ms_queued){
		util__decrement_send_quota(context);
	}
	if(dir == mosq_md_out && update){
		rc = db__message_write_inflight_out_latest(context);
		if(rc) return rc;
		rc = db__message_write_queued_out(context);
		if(rc) return rc;
	}
	return rc;
}