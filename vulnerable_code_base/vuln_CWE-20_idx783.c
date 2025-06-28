bool_t enc28j60IrqHandler(NetInterface *interface)
{
   bool_t flag;
   uint8_t status;
   flag = FALSE;
   enc28j60ClearBit(interface, ENC28J60_REG_EIE, EIE_INTIE);
   status = enc28j60ReadReg(interface, ENC28J60_REG_EIR);
   if((status & EIR_LINKIF) != 0)
   {
      enc28j60ClearBit(interface, ENC28J60_REG_EIE, EIE_LINKIE);
      interface->nicEvent = TRUE;
      flag |= osSetEventFromIsr(&netEvent);
   }
   if((status & EIR_PKTIF) != 0)
   {
      enc28j60ClearBit(interface, ENC28J60_REG_EIE, EIE_PKTIE);
      interface->nicEvent = TRUE;
      flag |= osSetEventFromIsr(&netEvent);
   }
   if((status & (EIR_TXIF | EIE_TXERIE)) != 0)
   {
      enc28j60ClearBit(interface, ENC28J60_REG_EIR, EIR_TXIF | EIE_TXERIE);
      flag |= osSetEventFromIsr(&interface->nicTxEvent);
   }
   enc28j60SetBit(interface, ENC28J60_REG_EIE, EIE_INTIE);
   return flag;
}