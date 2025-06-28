error_t mqttSnClientSendUnsubscribe(MqttSnClientContext *context,
   const char_t *topicName)
{
   error_t error;
   systime_t time;
   uint16_t topicId;
   MqttSnFlags flags;
   error = NO_ERROR;
   flags.all = 0;
   topicId = mqttSnClientFindPredefTopicName(context, topicName);
   if(topicId != MQTT_SN_INVALID_TOPIC_ID)
   {
      flags.topicIdType = MQTT_SN_PREDEFINED_TOPIC_ID;
   }
   else
   {
      if(osStrlen(topicName) == 2 && strchr(topicName, '#') == NULL &&
         strchr(topicName, '+') == NULL)
      {
         flags.topicIdType = MQTT_SN_SHORT_TOPIC_NAME;
      }
      else
      {
         flags.topicIdType = MQTT_SN_NORMAL_TOPIC_NAME;
      }
      error = mqttSnFormatUnsubscribe(&context->message, flags,
         context->msgId, topicId, topicName);
   }
   if(!error)
   {
      TRACE_INFO("Sending UNSUBSCRIBE message (%" PRIuSIZE " bytes)...\r\n",
         context->message.length);
      mqttSnDumpMessage(context->message.buffer, context->message.length);
      error = mqttSnClientSendDatagram(context, context->message.buffer,
         context->message.length);
      time = osGetSystemTime();
      context->retransmitStartTime = time;
      context->keepAliveTimestamp = time;
      context->state = MQTT_SN_CLIENT_STATE_SENDING_REQ;
      context->msgType = MQTT_SN_MSG_TYPE_UNSUBSCRIBE;
   }
   return error;
}