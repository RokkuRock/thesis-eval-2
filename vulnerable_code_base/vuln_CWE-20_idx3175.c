void rza1EthEventHandler(NetInterface *interface)
{
   error_t error;
   if((ETHER.EESR0 & ETHER_EESR0_FR) != 0)
   {
      ETHER.EESR0 = ETHER_EESR0_FR;
      do
      {
         error = rza1EthReceivePacket(interface);
      } while(error != ERROR_BUFFER_EMPTY);
   }
   ETHER.EESIPR0 =  ETHER_EESIPR0_TWBIP | ETHER_EESIPR0_FRIP;
}