bool_t ksz8851IrqHandler(NetInterface *interface)
{
   bool_t flag;
   size_t n;
   uint16_t ier;
   uint16_t isr;
   flag = FALSE;
   ier = ksz8851ReadReg(interface, KSZ8851_REG_IER);
   ksz8851WriteReg(interface, KSZ8851_REG_IER, 0);
   isr = ksz8851ReadReg(interface, KSZ8851_REG_ISR);
   if((isr & ISR_LCIS) != 0)
   {
      ier &= ~IER_LCIE;
      interface->nicEvent = TRUE;
      flag |= osSetEventFromIsr(&netEvent);
   }
   if((isr & ISR_TXIS) != 0)
   {
      ksz8851WriteReg(interface, KSZ8851_REG_ISR, ISR_TXIS);
      n = ksz8851ReadReg(interface, KSZ8851_REG_TXMIR) & TXMIR_TXMA_MASK;
      if(n >= (ETH_MAX_FRAME_SIZE + 8))
      {
         flag |= osSetEventFromIsr(&interface->nicTxEvent);
      }
   }
   if((isr & ISR_RXIS) != 0)
   {
      ier &= ~IER_RXIE;
      interface->nicEvent = TRUE;
      flag |= osSetEventFromIsr(&netEvent);
   }
   ksz8851WriteReg(interface, KSZ8851_REG_IER, ier);
   return flag;
}