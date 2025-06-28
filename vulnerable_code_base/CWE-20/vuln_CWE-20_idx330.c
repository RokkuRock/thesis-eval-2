bool_t dm9000IrqHandler(NetInterface *interface)
{
   bool_t flag;
   uint8_t status;
   uint8_t mask;
   Dm9000Context *context;
   flag = FALSE;
   context = (Dm9000Context *) interface->nicContext;
   status = dm9000ReadReg(DM9000_REG_ISR);
   if((status & ISR_LNKCHG) != 0)
   {
      mask = dm9000ReadReg(DM9000_REG_IMR);
      dm9000WriteReg(DM9000_REG_IMR, mask & ~IMR_LNKCHGI);
      interface->nicEvent = TRUE;
      flag |= osSetEventFromIsr(&netEvent);
   }
   if((status & ISR_PT) != 0)
   {
      if(dm9000ReadReg(DM9000_REG_NSR) & (NSR_TX2END | NSR_TX1END))
      {
         if(context->queuedPackets > 0)
         {
            context->queuedPackets--;
         }
         flag |= osSetEventFromIsr(&interface->nicTxEvent);
      }
      dm9000WriteReg(DM9000_REG_ISR, ISR_PT);
   }
   if((status & ISR_PR) != 0)
   {
      mask = dm9000ReadReg(DM9000_REG_IMR);
      dm9000WriteReg(DM9000_REG_IMR, mask & ~IMR_PRI);
      interface->nicEvent = TRUE;
      flag |= osSetEventFromIsr(&netEvent);
   }
   return flag;
}