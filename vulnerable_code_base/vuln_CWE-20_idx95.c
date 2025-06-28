error_t httpClientFormatAuthorizationField(HttpClientContext *context)
{
   size_t n;
   char_t *p;
   HttpClientAuthParams *authParams;
   if(context->bufferLen < 2 || context->bufferLen > HTTP_CLIENT_BUFFER_SIZE)
      return ERROR_INVALID_SYNTAX;
   authParams = &context->authParams;
#if (HTTP_CLIENT_BASIC_AUTH_SUPPORT == ENABLED)
   if(authParams->mode == HTTP_AUTH_MODE_BASIC)
   {
      size_t k;
      size_t m;
      n = osStrlen(authParams->username) + osStrlen(authParams->password);
      if((context->bufferLen + n + 22) > HTTP_CLIENT_BUFFER_SIZE)
         return ERROR_BUFFER_OVERFLOW;
      p = context->buffer + context->bufferLen - 2;
      n = osSprintf(p, "Authorization: Basic ");
      m = osSprintf(p + n, "%s:%s", authParams->username, authParams->password);
      base64Encode(p + n, m, NULL, &k);
      if((context->bufferLen + n + k) > HTTP_CLIENT_BUFFER_SIZE)
         return ERROR_BUFFER_OVERFLOW;
      base64Encode(p + n, m, p + n, &k);
      n += k;
      if((context->bufferLen + n + 2) > HTTP_CLIENT_BUFFER_SIZE)
         return ERROR_BUFFER_OVERFLOW;
      osSprintf(p + n, "\r\n\r\n");
      context->bufferLen = context->bufferLen + n + 2;
   }
   else
#endif
#if (HTTP_CLIENT_DIGEST_AUTH_SUPPORT == ENABLED)
   if(authParams->mode == HTTP_AUTH_MODE_DIGEST)
   {
      error_t error;
      const char_t *q;
      const char_t *uri;
      size_t uriLen;
      char_t response[HTTP_CLIENT_MAX_RESPONSE_LEN + 1];
      context->buffer[context->bufferLen] = '\0';
      q = strchr(context->buffer, ' ');
      if(q == NULL)
         return ERROR_INVALID_SYNTAX;
      uri = q + 1;
      q = strchr(uri, ' ');
      if(q == NULL)
         return ERROR_INVALID_SYNTAX;
      uriLen = q - uri;
      if(authParams->qop == HTTP_AUTH_QOP_AUTH ||
         authParams->qop == HTTP_AUTH_QOP_AUTH_INT)
      {
         if(context->randCallback == NULL)
            return ERROR_PRNG_NOT_READY;
         error = context->randCallback(authParams->cnonce, HTTP_CLIENT_CNONCE_SIZE);
         if(error)
            return error;
         httpEncodeHexString(authParams->cnonce, HTTP_CLIENT_CNONCE_SIZE,
            authParams->cnonce);
         authParams->nc++;
      }
      error = httpClientComputeDigest(authParams, context->method,
         osStrlen(context->method), uri, uriLen, response);
      if(error)
         return error;
      n = osStrlen(authParams->username) + osStrlen(authParams->realm) +
         uriLen + osStrlen(authParams->nonce) + osStrlen(authParams->cnonce) +
         osStrlen(response) + osStrlen(authParams->opaque);
      if((context->bufferLen + n + 121) > HTTP_CLIENT_BUFFER_SIZE)
         return ERROR_BUFFER_OVERFLOW;
      p = context->buffer + context->bufferLen - 2;
      n = osSprintf(p, "Authorization: Digest ");
      n += osSprintf(p + n, "username=\"%s\", ", authParams->username);
      n += osSprintf(p + n, "realm=\"%s\", ", authParams->realm);
      n += osSprintf(p + n, "uri=\"");
      osStrncpy(p + n, uri, uriLen);
      n += uriLen;
      n += osSprintf(p + n, "\", ");
      n += osSprintf(p + n, "nonce=\"%s\", ", authParams->nonce);
      if(authParams->qop == HTTP_AUTH_QOP_AUTH)
      {
         n += osSprintf(p + n, "qop=auth, ");
         n += osSprintf(p + n, "nc=%08x, ", authParams->nc);
         n += osSprintf(p + n, "cnonce=\"%s\", ", authParams->cnonce);
      }
      n += osSprintf(p + n, "response=\"%s\"", response);
      if(authParams->opaque[0] != '\0')
      {
         n += osSprintf(p + n, ", opaque=\"%s\"", authParams->opaque);
      }
      osSprintf(p + n, "\r\n\r\n");
      context->bufferLen = context->bufferLen + n + 2;
   }
   else
#endif
   {
   }
   return NO_ERROR;
}