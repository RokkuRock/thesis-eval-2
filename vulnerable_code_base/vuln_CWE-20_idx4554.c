error_t ksz8851ReceivePacket(NetInterface *interface)
{
   size_t n;
   uint16_t status;
   Ksz8851Context *context;
   NetRxAncillary ancillary;
   context = (Ksz8851Context *) interface->nicContext;
   status = ksz8851ReadReg(interface, KSZ8851_REG_RXFHSR);
   if((status & RXFHSR_RXFV) != 0)
   {
      if((status & (RXFHSR_RXMR | RXFHSR_RXFTL | RXFHSR_RXRF | RXFHSR_RXCE)) == 0)
      {
         n = ksz8851ReadReg(interface, KSZ8851_REG_RXFHBCR) & RXFHBCR_RXBC_MASK;
         if(n > 0 && n <= ETH_MAX_FRAME_SIZE)
         {
            ksz8851WriteReg(interface, KSZ8851_REG_RXFDPR, RXFDPR_RXFPAI);
            ksz8851SetBit(interface, KSZ8851_REG_RXQCR, RXQCR_SDA);
            ksz8851ReadFifo(interface, context->rxBuffer, n);
            ksz8851ClearBit(interface, KSZ8851_REG_RXQCR, RXQCR_SDA);
            ancillary = NET_DEFAULT_RX_ANCILLARY;
            nicProcessPacket(interface, context->rxBuffer, n, &ancillary);
            return NO_ERROR;
         }
      }
   }
   ksz8851SetBit(interface, KSZ8851_REG_RXQCR, RXQCR_RRXEF);
   return ERROR_INVALID_PACKET;
}