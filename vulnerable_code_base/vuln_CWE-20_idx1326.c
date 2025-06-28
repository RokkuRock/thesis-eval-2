error_t webSocketParseAuthenticateField(WebSocket *webSocket, char_t *value)
{
#if (WEB_SOCKET_BASIC_AUTH_SUPPORT == ENABLED || WEB_SOCKET_DIGEST_AUTH_SUPPORT == ENABLED)
   size_t n;
   char_t *p;
   char_t *token;
   char_t *separator;
   char_t *name;
   WebSocketAuthContext *authContext;
   authContext = &webSocket->authContext;
   token = osStrtok_r(value, " \t", &p);
   if(token == NULL)
      return ERROR_INVALID_SYNTAX;
   if(!osStrcasecmp(token, "Basic"))
   {
      authContext->requiredAuthMode = WS_AUTH_MODE_BASIC;
   }
   else if(!osStrcasecmp(token, "Digest"))
   {
      authContext->requiredAuthMode = WS_AUTH_MODE_DIGEST;
   }
   else
   {
      return ERROR_INVALID_SYNTAX;
   }
   token = osStrtok_r(NULL, ",", &p);
   while(token != NULL)
   {
      separator = strchr(token, '=');
      if(separator != NULL)
      {
         *separator = '\0';
         name = strTrimWhitespace(token);
         value = strTrimWhitespace(separator + 1);
         n = osStrlen(value);
         if(n > 0 && value[n - 1] == '\"')
            value[n - 1] = '\0';
         if(value[0] == '\"')
            value++;
         if(!osStrcasecmp(name, "realm"))
         {
            strSafeCopy(authContext->realm, value, WEB_SOCKET_REALM_MAX_LEN);
         }
#if (WEB_SOCKET_DIGEST_AUTH_SUPPORT == ENABLED)
         else if(!osStrcasecmp(name, "nonce"))
         {
            strSafeCopy(authContext->nonce, value, WEB_SOCKET_NONCE_MAX_LEN + 1);
         }
         else if(!osStrcasecmp(name, "opaque"))
         {
            strSafeCopy(authContext->opaque, value, WEB_SOCKET_OPAQUE_MAX_LEN + 1);
         }
         else if(!osStrcasecmp(name, "stale"))
         {
            if(!osStrcasecmp(value, "true"))
               authContext->stale = TRUE;
            else
               authContext->stale = FALSE;
         }
#endif
         token = osStrtok_r(NULL, ",", &p);
      }
   }
#endif
   return NO_ERROR;
}