error_t lpc546xxEthUpdateMacConfig(NetInterface *interface)
{
   uint32_t config;
   config = ENET->MAC_CONFIG;
   if(interface->linkSpeed == NIC_LINK_SPEED_100MBPS)
   {
      config |= ENET_MAC_CONFIG_FES_MASK;
   }
   else
   {
      config &= ~ENET_MAC_CONFIG_FES_MASK;
   }
   if(interface->duplexMode == NIC_FULL_DUPLEX_MODE)
   {
      config |= ENET_MAC_CONFIG_DM_MASK;
   }
   else
   {
      config &= ~ENET_MAC_CONFIG_DM_MASK;
   }
   ENET->MAC_CONFIG = config;
   return NO_ERROR;
}