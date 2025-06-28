int handle__publish(struct mosquitto *context)
{
	uint8_t dup;
	int rc = 0;
	int rc2;
	uint8_t header = context->in_packet.command;
	int res = 0;
	struct mosquitto_msg_store *msg, *stored = NULL;
	size_t len;
	uint16_t slen;
	char *topic_mount;
	mosquitto_property *properties = NULL;
	mosquitto_property *p, *p_prev;
	mosquitto_property *msg_properties_last;
	uint32_t message_expiry_interval = 0;
	int topic_alias = -1;
	uint8_t reason_code = 0;
	uint16_t mid = 0;
	if(context->state != mosq_cs_active){
		return MOSQ_ERR_PROTOCOL;
	}
	msg = mosquitto__calloc(1, sizeof(struct mosquitto_msg_store));
	if(msg == NULL){
		return MOSQ_ERR_NOMEM;
	}
	dup = (header & 0x08)>>3;
	msg->qos = (header & 0x06)>>1;
	if(dup == 1 && msg->qos == 0){
		log__printf(NULL, MOSQ_LOG_INFO,
				"Invalid PUBLISH (QoS=0 and DUP=1) from %s, disconnecting.", context->id);
		db__msg_store_free(msg);
		return MOSQ_ERR_MALFORMED_PACKET;
	}
	if(msg->qos == 3){
		log__printf(NULL, MOSQ_LOG_INFO,
				"Invalid QoS in PUBLISH from %s, disconnecting.", context->id);
		db__msg_store_free(msg);
		return MOSQ_ERR_MALFORMED_PACKET;
	}
	if(msg->qos > context->max_qos){
		log__printf(NULL, MOSQ_LOG_INFO,
				"Too high QoS in PUBLISH from %s, disconnecting.", context->id);
		db__msg_store_free(msg);
		return MOSQ_ERR_QOS_NOT_SUPPORTED;
	}
	msg->retain = (header & 0x01);
	if(msg->retain && db.config->retain_available == false){
		db__msg_store_free(msg);
		return MOSQ_ERR_RETAIN_NOT_SUPPORTED;
	}
	if(packet__read_string(&context->in_packet, &msg->topic, &slen)){
		db__msg_store_free(msg);
		return MOSQ_ERR_MALFORMED_PACKET;
	}
	if(!slen && context->protocol != mosq_p_mqtt5){
		db__msg_store_free(msg);
		return MOSQ_ERR_MALFORMED_PACKET;
	}
	if(msg->qos > 0){
		if(packet__read_uint16(&context->in_packet, &mid)){
			db__msg_store_free(msg);
			return MOSQ_ERR_MALFORMED_PACKET;
		}
		if(mid == 0){
			db__msg_store_free(msg);
			return MOSQ_ERR_PROTOCOL;
		}
		msg->source_mid = mid;
	}
	if(context->protocol == mosq_p_mqtt5){
		rc = property__read_all(CMD_PUBLISH, &context->in_packet, &properties);
		if(rc){
			db__msg_store_free(msg);
			return rc;
		}
		p = properties;
		p_prev = NULL;
		msg->properties = NULL;
		msg_properties_last = NULL;
		while(p){
			switch(p->identifier){
				case MQTT_PROP_CONTENT_TYPE:
				case MQTT_PROP_CORRELATION_DATA:
				case MQTT_PROP_PAYLOAD_FORMAT_INDICATOR:
				case MQTT_PROP_RESPONSE_TOPIC:
				case MQTT_PROP_USER_PROPERTY:
					if(msg->properties){
						msg_properties_last->next = p;
						msg_properties_last = p;
					}else{
						msg->properties = p;
						msg_properties_last = p;
					}
					if(p_prev){
						p_prev->next = p->next;
						p = p_prev->next;
					}else{
						properties = p->next;
						p = properties;
					}
					msg_properties_last->next = NULL;
					break;
				case MQTT_PROP_TOPIC_ALIAS:
					topic_alias = p->value.i16;
					p_prev = p;
					p = p->next;
					break;
				case MQTT_PROP_MESSAGE_EXPIRY_INTERVAL:
					message_expiry_interval = p->value.i32;
					p_prev = p;
					p = p->next;
					break;
				case MQTT_PROP_SUBSCRIPTION_IDENTIFIER:
					p_prev = p;
					p = p->next;
					break;
				default:
					p = p->next;
					break;
			}
		}
	}
	mosquitto_property_free_all(&properties);
	if(topic_alias == 0 || (context->listener && topic_alias > context->listener->max_topic_alias)){
		db__msg_store_free(msg);
		return MOSQ_ERR_TOPIC_ALIAS_INVALID;
	}else if(topic_alias > 0){
		if(msg->topic){
			rc = alias__add(context, msg->topic, (uint16_t)topic_alias);
			if(rc){
				db__msg_store_free(msg);
				return rc;
			}
		}else{
			rc = alias__find(context, &msg->topic, (uint16_t)topic_alias);
			if(rc){
				db__msg_store_free(msg);
				return MOSQ_ERR_PROTOCOL;
			}
		}
	}
#ifdef WITH_BRIDGE
	rc = bridge__remap_topic_in(context, &msg->topic);
	if(rc){
		db__msg_store_free(msg);
		return rc;
	}
#endif
	if(mosquitto_pub_topic_check(msg->topic) != MOSQ_ERR_SUCCESS){
		db__msg_store_free(msg);
		return MOSQ_ERR_MALFORMED_PACKET;
	}
	msg->payloadlen = context->in_packet.remaining_length - context->in_packet.pos;
	G_PUB_BYTES_RECEIVED_INC(msg->payloadlen);
	if(context->listener && context->listener->mount_point){
		len = strlen(context->listener->mount_point) + strlen(msg->topic) + 1;
		topic_mount = mosquitto__malloc(len+1);
		if(!topic_mount){
			db__msg_store_free(msg);
			return MOSQ_ERR_NOMEM;
		}
		snprintf(topic_mount, len, "%s%s", context->listener->mount_point, msg->topic);
		topic_mount[len] = '\0';
		mosquitto__free(msg->topic);
		msg->topic = topic_mount;
	}
	if(msg->payloadlen){
		if(db.config->message_size_limit && msg->payloadlen > db.config->message_size_limit){
			log__printf(NULL, MOSQ_LOG_DEBUG, "Dropped too large PUBLISH from %s (d%d, q%d, r%d, m%d, '%s', ... (%ld bytes))", context->id, dup, msg->qos, msg->retain, msg->source_mid, msg->topic, (long)msg->payloadlen);
			reason_code = MQTT_RC_PACKET_TOO_LARGE;
			goto process_bad_message;
		}
		msg->payload = mosquitto__malloc(msg->payloadlen+1);
		if(msg->payload == NULL){
			db__msg_store_free(msg);
			return MOSQ_ERR_NOMEM;
		}
		((uint8_t *)msg->payload)[msg->payloadlen] = 0;
		if(packet__read_bytes(&context->in_packet, msg->payload, msg->payloadlen)){
			db__msg_store_free(msg);
			return MOSQ_ERR_MALFORMED_PACKET;
		}
	}
	rc = mosquitto_acl_check(context, msg->topic, msg->payloadlen, msg->payload, msg->qos, msg->retain, MOSQ_ACL_WRITE);
	if(rc == MOSQ_ERR_ACL_DENIED){
		log__printf(NULL, MOSQ_LOG_DEBUG,
				"Denied PUBLISH from %s (d%d, q%d, r%d, m%d, '%s', ... (%ld bytes))",
				context->id, dup, msg->qos, msg->retain, msg->source_mid, msg->topic,
				(long)msg->payloadlen);
		reason_code = MQTT_RC_NOT_AUTHORIZED;
		goto process_bad_message;
	}else if(rc != MOSQ_ERR_SUCCESS){
		db__msg_store_free(msg);
		return rc;
	}
	log__printf(NULL, MOSQ_LOG_DEBUG, "Received PUBLISH from %s (d%d, q%d, r%d, m%d, '%s', ... (%ld bytes))", context->id, dup, msg->qos, msg->retain, msg->source_mid, msg->topic, (long)msg->payloadlen);
	if(!strncmp(msg->topic, "$CONTROL/", 9)){
#ifdef WITH_CONTROL
		rc = control__process(context, msg);
		db__msg_store_free(msg);
		return rc;
#else
		reason_code = MQTT_RC_IMPLEMENTATION_SPECIFIC;
		goto process_bad_message;
#endif
	}
	{
		rc = plugin__handle_message(context, msg);
		if(rc == MOSQ_ERR_ACL_DENIED){
			log__printf(NULL, MOSQ_LOG_DEBUG,
					"Denied PUBLISH from %s (d%d, q%d, r%d, m%d, '%s', ... (%ld bytes))",
					context->id, dup, msg->qos, msg->retain, msg->source_mid, msg->topic,
					(long)msg->payloadlen);
			reason_code = MQTT_RC_NOT_AUTHORIZED;
			goto process_bad_message;
		}else if(rc != MOSQ_ERR_SUCCESS){
			db__msg_store_free(msg);
			return rc;
		}
	}
	if(msg->qos > 0){
		db__message_store_find(context, msg->source_mid, &stored);
	}
	if(stored && msg->source_mid != 0 &&
			(stored->qos != msg->qos
			 || stored->payloadlen != msg->payloadlen
			 || strcmp(stored->topic, msg->topic)
			 || memcmp(stored->payload, msg->payload, msg->payloadlen) )){
		log__printf(NULL, MOSQ_LOG_WARNING, "Reused message ID %u from %s detected. Clearing from storage.", msg->source_mid, context->id);
		db__message_remove_incoming(context, msg->source_mid);
		stored = NULL;
	}
	if(!stored){
		if(msg->qos == 0
				|| db__ready_for_flight(context, mosq_md_in, msg->qos)
				|| db__ready_for_queue(context, msg->qos, &context->msgs_in)){
			dup = 0;
			rc = db__message_store(context, msg, message_expiry_interval, 0, mosq_mo_client);
			if(rc) return rc;
		}else{
			reason_code = MQTT_RC_QUOTA_EXCEEDED;
			goto process_bad_message;
		}
		stored = msg;
		msg = NULL;
	}else{
		db__msg_store_free(msg);
		msg = NULL;
		dup = 1;
	}
	switch(stored->qos){
		case 0:
			rc2 = sub__messages_queue(context->id, stored->topic, stored->qos, stored->retain, &stored);
			if(rc2 > 0) rc = 1;
			break;
		case 1:
			util__decrement_receive_quota(context);
			rc2 = sub__messages_queue(context->id, stored->topic, stored->qos, stored->retain, &stored);
			if(rc2 == MOSQ_ERR_SUCCESS || context->protocol != mosq_p_mqtt5){
				if(send__puback(context, mid, 0, NULL)) rc = 1;
			}else if(rc2 == MOSQ_ERR_NO_SUBSCRIBERS){
				if(send__puback(context, mid, MQTT_RC_NO_MATCHING_SUBSCRIBERS, NULL)) rc = 1;
			}else{
				rc = rc2;
			}
			break;
		case 2:
			if(dup == 0){
				res = db__message_insert(context, stored->source_mid, mosq_md_in, stored->qos, stored->retain, stored, NULL, false);
			}else{
				res = 0;
			}
			if(!res){
				if(send__pubrec(context, stored->source_mid, 0, NULL)) rc = 1;
			}else if(res == 1){
				rc = 1;
			}
			break;
	}
	db__message_write_queued_in(context);
	return rc;
process_bad_message:
	rc = 1;
	if(msg){
		switch(msg->qos){
			case 0:
				rc = MOSQ_ERR_SUCCESS;
				break;
			case 1:
				rc = send__puback(context, msg->source_mid, reason_code, NULL);
				break;
			case 2:
				rc = send__pubrec(context, msg->source_mid, reason_code, NULL);
				break;
		}
		db__msg_store_free(msg);
	}
	return rc;
}