void lpc546xxEthEnableIrq(NetInterface *interface)
{
   NVIC_EnableIRQ(ETHERNET_IRQn);
   if(interface->phyDriver != NULL)
   {
      interface->phyDriver->enableIrq(interface);
   }
   else if(interface->switchDriver != NULL)
   {
      interface->switchDriver->enableIrq(interface);
   }
   else
   {
   }
}