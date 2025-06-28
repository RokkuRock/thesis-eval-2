void httpClientParseQopParam(const HttpParam *param,
   HttpWwwAuthenticateHeader *authHeader)
{
#if (HTTP_CLIENT_DIGEST_AUTH_SUPPORT == ENABLED)
   size_t i;
   size_t n;
   authHeader->qop = HTTP_AUTH_QOP_NONE;
   for(i = 0; i < param->valueLen; i += (n + 1))
   {
      for(n = 0; (i + n) < param->valueLen; n++)
      {
         if(strchr(", \t", param->value[i + n]))
            break;
      }
      if(n == 4 && !osStrncasecmp(param->value + i, "auth", 4))
      {
         authHeader->qop = HTTP_AUTH_QOP_AUTH;
      }
   }
   if(authHeader->qop == HTTP_AUTH_QOP_NONE)
   {
      authHeader->mode = HTTP_AUTH_MODE_NONE;
   }
#endif
}