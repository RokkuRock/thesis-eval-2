error_t ndpCheckOptions(const uint8_t *options, size_t length)
{
   size_t i;
   NdpOption *option;
   i = 0;
   while((i + sizeof(NdpOption)) <= length)
   {
      option = (NdpOption *) (options + i);
      if(option->length == 0)
         return ERROR_INVALID_OPTION;
      i += option->length * 8;
   }
   return NO_ERROR;
}