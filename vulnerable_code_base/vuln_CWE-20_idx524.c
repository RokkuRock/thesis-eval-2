void esp32EthDisableIrq(NetInterface *interface)
{
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