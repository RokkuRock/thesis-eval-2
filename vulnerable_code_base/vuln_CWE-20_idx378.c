error_t ipv6AddRoute(const Ipv6Addr *prefix, uint_t prefixLen,
   NetInterface *interface, const Ipv6Addr *nextHop, uint_t metric)
{
   error_t error;
   uint_t i;
   Ipv6RoutingTableEntry *entry;
   Ipv6RoutingTableEntry *firstFreeEntry;
   if(prefix == NULL || interface == NULL)
      return ERROR_INVALID_PARAMETER;
   firstFreeEntry = NULL;
   osAcquireMutex(&netMutex);
   for(i = 0; i < IPV6_ROUTING_TABLE_SIZE; i++)
   {
      entry = &ipv6RoutingTable[i];
      if(entry->valid)
      {
         if(entry->prefixLen == prefixLen)
         {
            if(ipv6CompPrefix(&entry->prefix, prefix, prefixLen))
               break;
         }
      }
      else
      {
         if(firstFreeEntry == NULL)
            firstFreeEntry = entry;
      }
   }
   if(i >= IPV6_ROUTING_TABLE_SIZE)
      entry = firstFreeEntry;
   if(entry != NULL)
   {
      entry->prefix = *prefix;
      entry->prefixLen = prefixLen;
      entry->interface = interface;
      if(nextHop != NULL)
         entry->nextHop = *nextHop;
      else
         entry->nextHop = IPV6_UNSPECIFIED_ADDR;
      entry->metric = metric;
      entry->valid = TRUE;
      error = NO_ERROR;
   }
   else
   {
      error = ERROR_FAILURE;
   }
   osReleaseMutex(&netMutex);
   return error;
}