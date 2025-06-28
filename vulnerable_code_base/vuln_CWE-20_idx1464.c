error_t httpReadRequestHeader(HttpConnection *connection)
{
   error_t error;
   size_t length;
   error = socketSetTimeout(connection->socket, HTTP_SERVER_IDLE_TIMEOUT);
   if(error)
      return error;
   error = httpReceive(connection, connection->buffer,
      HTTP_SERVER_BUFFER_SIZE - 1, &length, SOCKET_FLAG_BREAK_CRLF);
   if(error)
      return error;
   error = socketSetTimeout(connection->socket, HTTP_SERVER_TIMEOUT);
   if(error)
      return error;
   connection->buffer[length] = '\0';
   TRACE_INFO("%s", connection->buffer);
   error = httpParseRequestLine(connection, connection->buffer);
   if(error)
      return error;
   connection->request.chunkedEncoding = FALSE;
   connection->request.contentLength = 0;
#if (HTTP_SERVER_WEB_SOCKET_SUPPORT == ENABLED)
   connection->request.upgradeWebSocket = FALSE;
   connection->request.connectionUpgrade = FALSE;
   osStrcpy(connection->request.clientKey, "");
#endif
   if(connection->request.version >= HTTP_VERSION_1_0)
   {
      char_t firstChar;
      char_t *separator;
      char_t *name;
      char_t *value;
      firstChar = '\0';
      while(1)
      {
         error = httpReadHeaderField(connection, connection->buffer,
            HTTP_SERVER_BUFFER_SIZE, &firstChar);
         if(error)
            return error;
         TRACE_DEBUG("%s", connection->buffer);
         if(!osStrcmp(connection->buffer, "\r\n"))
            break;
         separator = strchr(connection->buffer, ':');
         if(separator != NULL)
         {
            *separator = '\0';
            name = strTrimWhitespace(connection->buffer);
            value = strTrimWhitespace(separator + 1);
            httpParseHeaderField(connection, name, value);
         }
      }
   }
   if(connection->request.chunkedEncoding)
   {
      connection->request.byteCount = 0;
      connection->request.firstChunk = TRUE;
      connection->request.lastChunk = FALSE;
   }
   else
   {
      connection->request.byteCount = connection->request.contentLength;
   }
   return NO_ERROR;
}