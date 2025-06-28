error_t httpParseRequestLine(HttpConnection *connection, char_t *requestLine)
{
   error_t error;
   char_t *token;
   char_t *p;
   char_t *s;
   token = osStrtok_r(requestLine, " \r\n", &p);
   if(token == NULL)
      return ERROR_INVALID_REQUEST;
   error = strSafeCopy(connection->request.method, token, HTTP_SERVER_METHOD_MAX_LEN);
   if(error)
      return ERROR_INVALID_REQUEST;
   token = osStrtok_r(NULL, " \r\n", &p);
   if(token == NULL)
      return ERROR_INVALID_REQUEST;
   s = strchr(token, '?');
   if(s != NULL)
   {
      *s = '\0';
      error = httpDecodePercentEncodedString(token,
         connection->request.uri, HTTP_SERVER_URI_MAX_LEN);
      if(error)
         return ERROR_INVALID_REQUEST;
      if(osStrlen(s + 1) > HTTP_SERVER_QUERY_STRING_MAX_LEN)
         return ERROR_INVALID_REQUEST;
      osStrcpy(connection->request.queryString, s + 1);
   }
   else
   {
      error = httpDecodePercentEncodedString(token,
         connection->request.uri, HTTP_SERVER_URI_MAX_LEN);
      if(error)
         return ERROR_INVALID_REQUEST;
      connection->request.queryString[0] = '\0';
   }
   if(!osStrcasecmp(connection->request.uri, "/"))
      osStrcpy(connection->request.uri, connection->settings->defaultDocument);
   pathCanonicalize(connection->request.uri);
   token = osStrtok_r(NULL, " \r\n", &p);
   if(token == NULL)
   {
      connection->request.version = HTTP_VERSION_0_9;
      connection->request.keepAlive = FALSE;
   }
   else if(!osStrcasecmp(token, "HTTP/1.0"))
   {
      connection->request.version = HTTP_VERSION_1_0;
      connection->request.keepAlive = FALSE;
   }
   else if(!osStrcasecmp(token, "HTTP/1.1"))
   {
      connection->request.version = HTTP_VERSION_1_1;
      connection->request.keepAlive = TRUE;
   }
   else
   {
      return ERROR_INVALID_REQUEST;
   }
   return NO_ERROR;
}