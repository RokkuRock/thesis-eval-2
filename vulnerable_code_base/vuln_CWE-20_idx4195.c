void esp32EthEnableIrq(NetInterface *interface)
{
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