error_t httpParseParam(const char_t **pos, HttpParam *param)
{
   error_t error;
   size_t i;
   uint8_t c;
   bool_t escapeFlag;
   bool_t separatorFound;
   const char_t *p;
   if(pos == NULL || param == NULL)
      return ERROR_INVALID_PARAMETER;
   param->name = NULL;
   param->nameLen = 0;
   param->value = NULL;
   param->valueLen = 0;
   escapeFlag = FALSE;
   separatorFound = FALSE;
   error = ERROR_IN_PROGRESS;
   i = 0;
   p = *pos;
   while(error == ERROR_IN_PROGRESS)
   {
      c = (uint8_t) p[i];
      if(param->name == NULL)
      {
         if(c == '\0')
         {
            error = ERROR_NOT_FOUND;
         }
         else if(c == ' ' || c == '\t' || c == ',' || c == ';')
         {
         }
         else if(isalnum(c) || strchr("!#$%&'*+-.^_`|~", c) || c >= 128)
         {
            param->name = p + i;
         }
         else
         {
            error = ERROR_INVALID_SYNTAX;
         }
      }
      else if(param->nameLen == 0)
      {
         if(c == '\0' || c == ',' || c == ';')
         {
            param->nameLen = p + i - param->name;
            error = NO_ERROR;
         }
         else if(c == ' ' || c == '\t')
         {
            param->nameLen = p + i - param->name;
         }
         else if(c == '=')
         {
            separatorFound = TRUE;
            param->nameLen = p + i - param->name;
         }
         else if(isalnum(c) || strchr("!#$%&'*+-.^_`|~", c) || c >= 128)
         {
         }
         else
         {
            error = ERROR_INVALID_SYNTAX;
         }
      }
      else if(!separatorFound)
      {
         if(c == '\0' || c == ',' || c == ';')
         {
            error = NO_ERROR;
         }
         else if(c == ' ' || c == '\t')
         {
         }
         else if(c == '=')
         {
            separatorFound = TRUE;
         }
         else if(c == '\"')
         {
            i = param->name + param->nameLen - p;
            error = NO_ERROR;
         }
         else if(isalnum(c) || strchr("!#$%&'*+-.^_`|~", c) || c >= 128)
         {
            i = param->name + param->nameLen - p;
            error = NO_ERROR;
         }
         else
         {
            error = ERROR_INVALID_SYNTAX;
         }
      }
      else if(param->value == NULL)
      {
         if(c == '\0' || c == ',' || c == ';')
         {
            error = NO_ERROR;
         }
         else if(c == ' ' || c == '\t')
         {
         }
         else if(c == '\"')
         {
            param->value = p + i;
         }
         else if(isalnum(c) || strchr("!#$%&'*+-.^_`|~", c) || c >= 128)
         {
            param->value = p + i;
         }
         else
         {
            error = ERROR_INVALID_SYNTAX;
         }
      }
      else
      {
         if(param->value[0] == '\"')
         {
            if(c == '\0')
            {
               error = ERROR_INVALID_SYNTAX;
            }
            else if(escapeFlag)
            {
               escapeFlag = FALSE;
            }
            else if(c == '\\')
            {
               escapeFlag = TRUE;
            }
            else if(c == '\"')
            {
               i++;
               param->valueLen = p + i - param->value;
               error = NO_ERROR;
            }
            else if(isprint(c) || c == '\t' || c >= 128)
            {
            }
            else
            {
               error = ERROR_INVALID_SYNTAX;
            }
         }
         else
         {
            if(c == '\0' || c == ' ' || c == '\t' || c == ',' || c == ';')
            {
               param->valueLen = p + i - param->value;
               error = NO_ERROR;
            }
            else if(isalnum(c) || strchr("!#$%&'*+-.^_`|~", c) || c >= 128)
            {
            }
            else
            {
               error = ERROR_INVALID_SYNTAX;
            }
         }
      }
      if(error == ERROR_IN_PROGRESS)
         i++;
   }
   if(param->valueLen >= 2 && param->value[0] == '\"')
   {
      param->value++;
      param->valueLen -= 2;
   }
   *pos = p + i;
   return error;
}