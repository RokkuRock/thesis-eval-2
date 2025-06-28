error_t mqttSnClientSubscribe(MqttSnClientContext *context,
   const char_t *topicName, MqttSnQosLevel qos)
{
   error_t error;
   systime_t time;
   if(context == NULL || topicName == NULL)
      return ERROR_INVALID_PARAMETER;
   error = NO_ERROR;
   while(!error)
   {
      time = osGetSystemTime();
      if(context->state == MQTT_SN_CLIENT_STATE_ACTIVE)
      {
         mqttSnClientGenerateMessageId(context);
         context->startTime = time;
         error = mqttSnClientSendSubscribe(context, topicName, qos);
      }
      else if(context->state == MQTT_SN_CLIENT_STATE_SENDING_REQ)
      {
         if(timeCompare(time, context->startTime + context->timeout) >= 0)
         {
            context->state = MQTT_SN_CLIENT_STATE_DISCONNECTING;
            error = ERROR_TIMEOUT;
         }
         else if(timeCompare(time, context->retransmitStartTime +
            MQTT_SN_CLIENT_RETRY_TIMEOUT) >= 0)
         {
            error = mqttSnClientSendSubscribe(context, topicName, qos);
         }
         else
         {
            error = mqttSnClientProcessEvents(context, MQTT_SN_CLIENT_TICK_INTERVAL);
         }
      }
      else if(context->state == MQTT_SN_CLIENT_STATE_RESP_RECEIVED)
      {
         context->state = MQTT_SN_CLIENT_STATE_ACTIVE;
         if(context->msgType == MQTT_SN_MSG_TYPE_SUBACK)
         {
            if(context->returnCode == MQTT_SN_RETURN_CODE_ACCEPTED)
            {
               if(strchr(topicName, '#') == NULL && strchr(topicName, '+') == NULL)
               {
                  error = mqttSnClientAddTopic(context, topicName, context->topicId);
               }
               break;
            }
            else
            {
               error = ERROR_REQUEST_REJECTED;
            }
         }
         else
         {
            error = ERROR_UNEXPECTED_RESPONSE;
         }
      }
      else
      {
         error = ERROR_NOT_CONNECTED;
      }
   }
   return error;
}