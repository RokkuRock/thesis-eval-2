error_t ipv6ComputeSolicitedNodeAddr(const Ipv6Addr *ipAddr,
   Ipv6Addr *solicitedNodeAddr)
{
   error_t error;
   if(!ipv6IsMulticastAddr(ipAddr))
   {
      ipv6CopyAddr(solicitedNodeAddr, &IPV6_SOLICITED_NODE_ADDR_PREFIX);
      solicitedNodeAddr->b[13] = ipAddr->b[13];
      solicitedNodeAddr->b[14] = ipAddr->b[14];
      solicitedNodeAddr->b[15] = ipAddr->b[15];
      error = NO_ERROR;
   }
   else
   {
      error = ERROR_INVALID_ADDRESS;
   }
   return error;
}