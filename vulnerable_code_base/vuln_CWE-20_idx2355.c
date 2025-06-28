bool_t mqttSnClientIsShortTopicName(const char_t *topicName)
{
   bool_t res;
   res = FALSE;
   if(osStrlen(topicName) == 2)
   {
      if(strchr(topicName, '#') == NULL && strchr(topicName, '+') == NULL)
      {
         res = TRUE;
      }
   }
   return res;
}