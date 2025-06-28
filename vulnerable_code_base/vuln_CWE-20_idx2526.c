void ksz8851EventHandler(NetInterface *interface)
{
   uint16_t status;
   uint_t frameCount;
   status = ksz8851ReadReg(interface, KSZ8851_REG_ISR);
   if((status & ISR_LCIS) != 0)
   {
      ksz8851WriteReg(interface, KSZ8851_REG_ISR, ISR_LCIS);
      status = ksz8851ReadReg(interface, KSZ8851_REG_P1SR);
      if((status & P1SR_LINK_GOOD) != 0)
      {
         if((status & P1SR_OPERATION_SPEED) != 0)
         {
            interface->linkSpeed = NIC_LINK_SPEED_100MBPS;
         }
         else
         {
            interface->linkSpeed = NIC_LINK_SPEED_10MBPS;
         }
         if((status & P1SR_OPERATION_DUPLEX) != 0)
         {
            interface->duplexMode = NIC_FULL_DUPLEX_MODE;
         }
         else
         {
            interface->duplexMode = NIC_HALF_DUPLEX_MODE;
         }
         interface->linkState = TRUE;
      }
      else
      {
         interface->linkState = FALSE;
      }
      nicNotifyLinkChange(interface);
   }
   if((status & ISR_RXIS) != 0)
   {
      ksz8851WriteReg(interface, KSZ8851_REG_ISR, ISR_RXIS);
      frameCount = MSB(ksz8851ReadReg(interface, KSZ8851_REG_RXFCTR));
      while(frameCount > 0)
      {
         ksz8851ReceivePacket(interface);
         frameCount--;
      }
   }
   ksz8851SetBit(interface, KSZ8851_REG_IER, IER_LCIE | IER_RXIE);
}