DhcpOption *dhcpGetOption(const DhcpMessage *message,
   size_t length, uint8_t optionCode)
{
   uint_t i;
   DhcpOption *option;
   if(length < sizeof(DhcpMessage))
      return NULL;
   length -= sizeof(DhcpMessage);
   for(i = 0; i < length; i++)
   {
      option = (DhcpOption *) (message->options + i);
      if(option->code == DHCP_OPT_PAD)
         continue;
      if(option->code == DHCP_OPT_END)
         break;
      if((i + 1) >= length || (i + 1 + option->length) >= length)
         break;
      if(option->code == optionCode)
         return option;
      i += option->length + 1;
   }
   return NULL;
}