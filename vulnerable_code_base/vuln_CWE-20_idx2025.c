error_t httpClientSetUri(HttpClientContext *context, const char_t *uri)
{
   size_t m;
   size_t n;
   char_t *p;
   char_t *q;
   if(context == NULL || uri == NULL)
      return ERROR_INVALID_PARAMETER;
   if(uri[0] == '\0')
      return ERROR_INVALID_PARAMETER;
   if(context->requestState != HTTP_REQ_STATE_FORMAT_HEADER)
      return ERROR_WRONG_STATE;
   if(context->bufferLen > HTTP_CLIENT_BUFFER_SIZE)
      return ERROR_INVALID_SYNTAX;
   context->buffer[context->bufferLen] = '\0';
   p = strchr(context->buffer, ' ');
   if(p == NULL)
      return ERROR_INVALID_SYNTAX;
   p++;
   q = strpbrk(p, " ?");
   if(q == NULL)
      return ERROR_INVALID_SYNTAX;
   m = q - p;
   n = osStrlen(uri);
   if((context->bufferLen + n - m) > HTTP_CLIENT_BUFFER_SIZE)
      return ERROR_BUFFER_OVERFLOW;
   osMemmove(p + n, q, context->buffer + context->bufferLen + 1 - q);
   osStrncpy(p, uri, n);
   context->bufferLen = context->bufferLen + n - m;
   return NO_ERROR;
}