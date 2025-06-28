error_t mqttSnClientDisconnect(MqttSnClientContext *context)
{
   error_t error;
   systime_t time;
   if(context == NULL)
      return ERROR_INVALID_PARAMETER;
   error = NO_ERROR;
   while(!error)
   {
      time = osGetSystemTime();
      if(context->state == MQTT_SN_CLIENT_STATE_ACTIVE)
      {
         context->startTime = time;
         error = mqttSnClientSendDisconnect(context, 0);
      }
      else if(context->state == MQTT_SN_CLIENT_STATE_SENDING_REQ)
      {
         if(timeCompare(time, context->startTime + context->timeout) >= 0)
         {
            mqttSnClientShutdownConnection(context);
            error = ERROR_TIMEOUT;
         }
         else if(timeCompare(time, context->retransmitStartTime +
            MQTT_SN_CLIENT_RETRY_TIMEOUT) >= 0)
         {
            error = mqttSnClientSendDisconnect(context, 0);
         }
         else
         {
            error = mqttSnClientProcessEvents(context, MQTT_SN_CLIENT_TICK_INTERVAL);
         }
      }
      else if(context->state == MQTT_SN_CLIENT_STATE_DISCONNECTING)
      {
         error = mqttSnClientShutdownConnection(context);
         mqttSnClientCloseConnection(context);
         context->state = MQTT_SN_CLIENT_STATE_DISCONNECTED;
      }
      else if(context->state == MQTT_SN_CLIENT_STATE_DISCONNECTED)
      {
         break;
      }
      else
      {
         error = ERROR_WRONG_STATE;
      }
   }
   if(error != NO_ERROR && error != ERROR_WOULD_BLOCK)
   {
      mqttSnClientCloseConnection(context);
      context->state = MQTT_SN_CLIENT_STATE_DISCONNECTED;
   }
   return error;
}