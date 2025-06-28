error_t httpClientSetQueryString(HttpClientContext *context,
   const char_t *queryString)
{
   size_t m;
   size_t n;
   char_t *p;
   char_t *q;
   if(context == NULL || queryString == NULL)
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
      q = strchr(p + 1, ' ');
      if(q == NULL)
         return ERROR_INVALID_SYNTAX;
      m = q - p;
   }
   else
   {
      q = p;
      m = 0;
   }
   n = osStrlen(queryString);
   if(n == 0)
   {
      osMemmove(p, p + m, context->buffer + context->bufferLen + 1 - q);
   }
   else
   {
      n++;
      if((context->bufferLen + n - m) > HTTP_CLIENT_BUFFER_SIZE)
         return ERROR_BUFFER_OVERFLOW;
      osMemmove(p + n, q, context->buffer + context->bufferLen + 1 - q);
      p[0] = '?';
      osStrncpy(p + 1, queryString, n - 1);
   }
   context->bufferLen = context->bufferLen + n - m;
   return NO_ERROR;
}