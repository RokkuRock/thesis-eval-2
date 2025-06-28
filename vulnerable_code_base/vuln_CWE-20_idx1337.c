error_t httpClientParseHeaderField(HttpClientContext *context, char_t *line,
   size_t length)
{
   error_t error;
   char_t *name;
   size_t nameLen;
   char_t *value;
   size_t valueLen;
   char_t *separator;
   line[length] = '\0';
   TRACE_DEBUG("%s\r\n", line);
   error = httpCheckCharset(line, length, HTTP_CHARSET_TEXT);
   if(error)
      return error;
   if(line[0] == ' ' || line[0] == '\t')
   {
      if(context->bufferPos == 0)
         return ERROR_INVALID_SYNTAX;
      value = strTrimWhitespace(line);
      valueLen = osStrlen(value);
      if(valueLen > 0)
      {
         context->buffer[context->bufferPos - 1] = ' ';
         osMemmove(context->buffer + context->bufferPos, value, valueLen + 1);
         context->bufferLen = context->bufferPos + valueLen + 1;
      }
   }
   else
   {
      separator = strchr(line, ':');
      if(separator == NULL)
         return ERROR_INVALID_SYNTAX;
      *separator = '\0';
      name = strTrimWhitespace(line);
      value = strTrimWhitespace(separator + 1);
      nameLen = osStrlen(name);
      valueLen = osStrlen(value);
      if(nameLen == 0)
         return ERROR_INVALID_SYNTAX;
      if(!osStrcasecmp(name, "Connection"))
      {
         httpClientParseConnectionField(context, value);
      }
      else if(!osStrcasecmp(name, "Transfer-Encoding"))
      {
         httpClientParseTransferEncodingField(context, value);
      }
      else if(!osStrcasecmp(name, "Content-Length"))
      {
         httpClientParseContentLengthField(context, value);
      }
#if (HTTP_CLIENT_AUTH_SUPPORT == ENABLED)
      else if(!osStrcasecmp(name, "WWW-Authenticate"))
      {
         httpClientParseWwwAuthenticateField(context, value);
      }
#endif
      else
      {
      }
      osMemmove(context->buffer + context->bufferPos, name, nameLen + 1);
      osMemmove(context->buffer + context->bufferPos + nameLen + 1, value,
         valueLen + 1);
      context->bufferLen = context->bufferPos + nameLen + valueLen + 2;
   }
   context->bufferPos = context->bufferLen;
   return NO_ERROR;
}