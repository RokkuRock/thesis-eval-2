void lpc546xxEthTick(NetInterface *interface)
{
   if(interface->phyDriver != NULL)
   {
      interface->phyDriver->tick(interface);
   }
   else if(interface->switchDriver != NULL)
   {
      interface->switchDriver->tick(interface);
   }
   else
   {
   }
}