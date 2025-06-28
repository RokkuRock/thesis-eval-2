error_t httpCheckCharset(const char_t *s, size_t length, uint_t charset)
{
   error_t error;
   size_t i;
   uint8_t c;
   uint_t m;
   error = NO_ERROR;
   for(i = 0; i < length; i++)
   {
      c = (uint8_t) s[i];
      m = HTTP_CHARSET_OCTET;
      if(iscntrl(c))
         m |= HTTP_CHARSET_CTL;
      if(isprint(c) && c <= 126)
         m |= HTTP_CHARSET_TEXT | HTTP_CHARSET_VCHAR;
      if(c == ' ' || c == '\t')
         m |= HTTP_CHARSET_TEXT | HTTP_CHARSET_LWS;
      if(isalpha(c))
         m |= HTTP_CHARSET_TCHAR | HTTP_CHARSET_ALPHA;
      if(osIsdigit(c))
         m |= HTTP_CHARSET_TCHAR | HTTP_CHARSET_DIGIT;
      if(isxdigit(c))
         m |= HTTP_CHARSET_HEX;
      if(c >= 128)
         m |= HTTP_CHARSET_TEXT | HTTP_CHARSET_OBS_TEXT;
      if(strchr("!#$%&'*+-.^_`|~", c))
         m |= HTTP_CHARSET_TCHAR;
      if((m & charset) == 0)
         error = ERROR_INVALID_SYNTAX;
   }
   return error;
}