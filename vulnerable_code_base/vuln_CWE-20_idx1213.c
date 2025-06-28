void *ndpGetOption(uint8_t *options, size_t length, uint8_t type)
{
   size_t i;
   NdpOption *option;
   i = 0;
   while((i + sizeof(NdpOption)) <= length)
   {
      option = (NdpOption *) (options + i);
      if(option->length == 0)
         break;
      if((i + option->length * 8) > length)
         break;
      if(option->type == type || type == NDP_OPT_ANY)
         return option;
      i += option->length * 8;
   }
   return NULL;
}