error_t httpClientSetMethod(HttpClientContext *context, const char_t *method)
{
   size_t m;
   size_t n;
   char_t *p;
   if(context == NULL || method == NULL)
      return ERROR_INVALID_PARAMETER;
   n = osStrlen(method);
   if(n == 0 || n > HTTP_CLIENT_MAX_METHOD_LEN)
      return ERROR_INVALID_LENGTH;
   if(context->bufferLen > HTTP_CLIENT_BUFFER_SIZE)
      return ERROR_INVALID_SYNTAX;
   context->buffer[context->bufferLen] = '\0';
   p = strchr(context->buffer, ' ');
   if(p == NULL)
      return ERROR_INVALID_SYNTAX;
   m = p - context->buffer;
   if((context->bufferLen + n - m) > HTTP_CLIENT_BUFFER_SIZE)
      return ERROR_BUFFER_OVERFLOW;
   osMemmove(context->buffer + n, p, context->bufferLen + 1 - m);
   osStrncpy(context->buffer, method, n);
   context->bufferLen = context->bufferLen + n - m;
   osStrcpy(context->method, method);
   return NO_ERROR;
}