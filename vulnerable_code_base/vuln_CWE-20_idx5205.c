error_t ftpClientParseDirEntry(char_t *line, FtpDirEntry *dirEntry)
{
   uint_t i;
   size_t n;
   char_t *p;
   char_t *token;
   static const char_t months[13][4] =
   {
      "   ",
      "Jan",
      "Feb",
      "Mar",
      "Apr",
      "May",
      "Jun",
      "Jul",
      "Aug",
      "Sep",
      "Oct",
      "Nov",
      "Dec"
   };
   token = osStrtok_r(line, " \t", &p);
   if(token == NULL)
      return ERROR_INVALID_SYNTAX;
   if(osIsdigit(token[0]))
   {
      if(osStrlen(token) == 8 && token[2] == '-' && token[5] == '-')
      {
         dirEntry->modified.month = (uint8_t) osStrtoul(token, NULL, 10);
         dirEntry->modified.day = (uint8_t) osStrtoul(token + 3, NULL, 10);
         dirEntry->modified.year = (uint16_t) osStrtoul(token + 6, NULL, 10) + 2000;
      }
      else if(osStrlen(token) == 10 && token[2] == '/' && token[5] == '/')
      {
         dirEntry->modified.month = (uint8_t) osStrtoul(token, NULL, 10);
         dirEntry->modified.day = (uint8_t) osStrtoul(token + 3, NULL, 10);
         dirEntry->modified.year = (uint16_t) osStrtoul(token + 6, NULL, 10);
      }
      else
      {
         return ERROR_INVALID_SYNTAX;
      }
      token = osStrtok_r(NULL, " ", &p);
      if(token == NULL)
         return ERROR_INVALID_SYNTAX;
      if(osStrlen(token) >= 5 && token[2] == ':')
      {
         dirEntry->modified.hours = (uint8_t) osStrtoul(token, NULL, 10);
         dirEntry->modified.minutes = (uint8_t) osStrtoul(token + 3, NULL, 10);
         if(strstr(token, "PM") != NULL)
            dirEntry->modified.hours += 12;
      }
      else
      {
         return ERROR_INVALID_SYNTAX;
      }
      token = osStrtok_r(NULL, " ", &p);
      if(token == NULL)
         return ERROR_INVALID_SYNTAX;
      if(!osStrcmp(token, "<DIR>"))
      {
         dirEntry->attributes |= FTP_FILE_ATTR_DIRECTORY;
      }
      else
      {
         dirEntry->size = osStrtoul(token, NULL, 10);
      }
      token = osStrtok_r(NULL, " \r\n", &p);
      if(token == NULL)
         return ERROR_INVALID_SYNTAX;
      n = osStrlen(token);
      n = MIN(n, FTP_CLIENT_MAX_FILENAME_LEN);
      osStrncpy(dirEntry->name, token, n);
      dirEntry->name[n] = '\0';
   }
   else
   {
      if(strchr(token, 'd') != NULL)
         dirEntry->attributes |= FTP_FILE_ATTR_DIRECTORY;
      if(strchr(token, 'w') == NULL)
         dirEntry->attributes |= FTP_FILE_ATTR_READ_ONLY;
      token = osStrtok_r(NULL, " ", &p);
      if(token == NULL)
         return ERROR_INVALID_SYNTAX;
      token = osStrtok_r(NULL, " ", &p);
      if(token == NULL)
         return ERROR_INVALID_SYNTAX;
      token = osStrtok_r(NULL, " ", &p);
      if(token == NULL)
         return ERROR_INVALID_SYNTAX;
      token = osStrtok_r(NULL, " ", &p);
      if(token == NULL)
         return ERROR_INVALID_SYNTAX;
      dirEntry->size = osStrtoul(token, NULL, 10);
      token = osStrtok_r(NULL, " ", &p);
      if(token == NULL)
         return ERROR_INVALID_SYNTAX;
      for(i = 1; i <= 12; i++)
      {
         if(!osStrcmp(token, months[i]))
         {
            dirEntry->modified.month = i;
            break;
         }
      }
      token = osStrtok_r(NULL, " ", &p);
      if(token == NULL)
         return ERROR_INVALID_SYNTAX;
      dirEntry->modified.day = (uint8_t) osStrtoul(token, NULL, 10);
      token = osStrtok_r(NULL, " ", &p);
      if(token == NULL)
         return ERROR_INVALID_SYNTAX;
      if(osStrlen(token) == 4)
      {
         dirEntry->modified.year = (uint16_t) osStrtoul(token, NULL, 10);
      }
      else if(osStrlen(token) == 5)
      {
         token[2] = '\0';
         dirEntry->modified.hours = (uint8_t) osStrtoul(token, NULL, 10);
         dirEntry->modified.minutes = (uint8_t) osStrtoul(token + 3, NULL, 10);
      }
      else
      {
         return ERROR_INVALID_SYNTAX;
      }
      token = osStrtok_r(NULL, " \r\n", &p);
      if(token == NULL)
         return ERROR_INVALID_SYNTAX;
      n = osStrlen(token);
      n = MIN(n, FTP_CLIENT_MAX_FILENAME_LEN);
      osStrncpy(dirEntry->name, token, n);
      dirEntry->name[n] = '\0';
   }
   return NO_ERROR;
}