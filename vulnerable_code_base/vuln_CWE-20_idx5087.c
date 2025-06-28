void dhcpAddOption(DhcpMessage *message, uint8_t optionCode,
   const void *optionValue, size_t optionLen)
{
   size_t n;
   DhcpOption *option;
   n = 0;
   while(1)
   {
      option = (DhcpOption *) (message->options + n);
      if(option->code == DHCP_OPT_END)
         break;
      n += sizeof(DhcpOption) + option->length;
   }
   if(optionLen <= UINT8_MAX)
   {
      option = (DhcpOption *) (message->options + n);
      option->code = optionCode;
      option->length = (uint8_t) optionLen;
      osMemcpy(option->value, optionValue, optionLen);
      n += sizeof(DhcpOption) + option->length;
      option = (DhcpOption *) (message->options + n);
      option->code = DHCP_OPT_END;
   }
}