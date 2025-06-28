error_t httpClientAddQueryParam(HttpClientContext *context,
   const char_t *name, const char_t *value)
{
   size_t nameLen;
   size_t valueLen;
   char_t separator;
   char_t *p;
   if(context == NULL || name == NULL)
      return ERROR_INVALID_PARAMETER;
   if(name[0] == '\0')
      return ERROR_INVALID_PARAMETER;
   if(context->requestState != HTTP_REQ_STATE_FORMAT_HEADER)
      return ERROR_WRONG_STATE;
   if(context->bufferLen > HTTP_CLIENT_BUFFER_SIZE)
      return ERROR_INVALID_SYNTAX;
   context->buffer[context->bufferLen] = '\0';
   p = strchr(context->buffer, ' ');
   if(p == NULL)
      return ERROR_INVALID_SYNTAX;
   p = strpbrk(p + 1, " ?");
   if(p == NULL)
      return ERROR_INVALID_SYNTAX;
   if(*p == '?')
   {
      p = strchr(p + 1, ' ');
      if(p == NULL)
         return ERROR_INVALID_SYNTAX;
      separator = '&';
   }
   else
   {
      separator = '?';
   }
   nameLen = osStrlen(name);
   if(value == NULL)
   {
      if((context->bufferLen + nameLen + 1) > HTTP_CLIENT_BUFFER_SIZE)
         return ERROR_BUFFER_OVERFLOW;
      osMemmove(p + nameLen + 1, p, context->buffer + context->bufferLen + 1 - p);
      p[0] = separator;
      osStrncpy(p + 1, name, nameLen);
      context->bufferLen += nameLen + 1;
   }
   else
   {
      valueLen = osStrlen(value);
      if((context->bufferLen + nameLen + valueLen + 2) > HTTP_CLIENT_BUFFER_SIZE)
         return ERROR_BUFFER_OVERFLOW;
      osMemmove(p + nameLen + valueLen + 2, p, context->buffer +
         context->bufferLen + 1 - p);
      p[0] = separator;
      osStrncpy(p + 1, name, nameLen);
      p[nameLen + 1] = '=';
      osStrncpy(p + nameLen + 2, value, valueLen);
      context->bufferLen += nameLen + valueLen + 2;
   }
   return NO_ERROR;
}