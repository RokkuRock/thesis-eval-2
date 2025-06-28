bool_t enc624j600IrqHandler(NetInterface *interface)
{
   bool_t flag;
   uint16_t status;
   flag = FALSE;
   enc624j600ClearBit(interface, ENC624J600_REG_EIE, EIE_INTIE);
   status = enc624j600ReadReg(interface, ENC624J600_REG_EIR);
   if((status & EIR_LINKIF) != 0)
   {
      enc624j600ClearBit(interface, ENC624J600_REG_EIE, EIE_LINKIE);
      interface->nicEvent = TRUE;
      flag |= osSetEventFromIsr(&netEvent);
   }
   if((status & EIR_PKTIF) != 0)
   {
      enc624j600ClearBit(interface, ENC624J600_REG_EIE, EIE_PKTIE);
      interface->nicEvent = TRUE;
      flag |= osSetEventFromIsr(&netEvent);
   }
   if((status & (EIR_TXIF | EIR_TXABTIF)) != 0)
   {
      enc624j600ClearBit(interface, ENC624J600_REG_EIR, EIR_TXIF | EIR_TXABTIF);
      flag |= osSetEventFromIsr(&interface->nicTxEvent);
   }
   enc624j600SetBit(interface, ENC624J600_REG_EIE, EIE_INTIE);
   return flag;
}