void lpc546xxEthDisableIrq(NetInterface *interface)
{
   NVIC_DisableIRQ(ETHERNET_IRQn);
   if(interface->phyDriver != NULL)
   {
      interface->phyDriver->disableIrq(interface);
   }
   else if(interface->switchDriver != NULL)
   {
      interface->switchDriver->disableIrq(interface);
   }
   else
   {
   }
}