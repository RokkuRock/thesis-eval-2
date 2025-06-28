error_t ftpClientParsePwdReply(FtpClientContext *context, char_t *path,
   size_t maxLen)
{
   size_t length;
   char_t *p;
   p = strrchr(context->buffer, '\"');
   if(p == NULL)
      return ERROR_INVALID_SYNTAX;
   *p = '\0';
   p = strchr(context->buffer, '\"');
   if(p == NULL)
      return ERROR_INVALID_SYNTAX;
   length = osStrlen(p + 1);
   length = MIN(length, maxLen);
   osStrncpy(path, p + 1, length);
   path[length] = '\0';
   return NO_ERROR;
}