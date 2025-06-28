error_t lpc546xxEthUpdateMacAddrFilter(NetInterface *interface)
{
   uint_t i;
   bool_t acceptMulticast;
   TRACE_DEBUG("Updating MAC filter...\r\n");
   ENET->MAC_ADDR_LOW = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   ENET->MAC_ADDR_HIGH = interface->macAddr.w[2];
   acceptMulticast = FALSE;
   for(i = 0; i < MAC_ADDR_FILTER_SIZE; i++)
   {
      if(interface->macAddrFilter[i].refCount > 0)
      {
         acceptMulticast = TRUE;
         break;
      }
   }
   if(acceptMulticast)
   {
      ENET->MAC_FRAME_FILTER |= ENET_MAC_FRAME_FILTER_PM_MASK;
   }
   else
   {
      ENET->MAC_FRAME_FILTER &= ~ENET_MAC_FRAME_FILTER_PM_MASK;
   }
   return NO_ERROR;
}