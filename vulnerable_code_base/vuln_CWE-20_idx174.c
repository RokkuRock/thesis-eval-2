error_t ipStringToAddr(const char_t *str, IpAddr *ipAddr)
{
   error_t error;
#if (IPV6_SUPPORT == ENABLED)
   if(strchr(str, ':'))
   {
      ipAddr->length = sizeof(Ipv6Addr);
      error = ipv6StringToAddr(str, &ipAddr->ipv6Addr);
   }
   else
#endif
#if (IPV4_SUPPORT == ENABLED)
   if(strchr(str, '.'))
   {
      ipAddr->length = sizeof(Ipv4Addr);
      error = ipv4StringToAddr(str, &ipAddr->ipv4Addr);
   }
   else
#endif
   {
      error = ERROR_FAILURE;
   }
   return error;
}