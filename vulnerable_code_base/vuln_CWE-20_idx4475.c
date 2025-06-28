error_t enc28j60SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t length;
   length = netBufferGetLength(buffer) - offset;
   if(length > 1536)
   {
      osSetEvent(&interface->nicTxEvent);
      return ERROR_INVALID_LENGTH;
   }
   if(!interface->linkState)
   {
      osSetEvent(&interface->nicTxEvent);
      return NO_ERROR;
   }
   enc28j60SetBit(interface, ENC28J60_REG_ECON1, ECON1_TXRST);
   enc28j60ClearBit(interface, ENC28J60_REG_ECON1, ECON1_TXRST);
   enc28j60ClearBit(interface, ENC28J60_REG_EIR, EIR_TXIF | EIR_TXERIF);
   enc28j60WriteReg(interface, ENC28J60_REG_ETXSTL, LSB(ENC28J60_TX_BUFFER_START));
   enc28j60WriteReg(interface, ENC28J60_REG_ETXSTH, MSB(ENC28J60_TX_BUFFER_START));
   enc28j60WriteReg(interface, ENC28J60_REG_EWRPTL, LSB(ENC28J60_TX_BUFFER_START));
   enc28j60WriteReg(interface, ENC28J60_REG_EWRPTH, MSB(ENC28J60_TX_BUFFER_START));
   enc28j60WriteBuffer(interface, buffer, offset);
   enc28j60WriteReg(interface, ENC28J60_REG_ETXNDL, LSB(ENC28J60_TX_BUFFER_START + length));
   enc28j60WriteReg(interface, ENC28J60_REG_ETXNDH, MSB(ENC28J60_TX_BUFFER_START + length));
   enc28j60SetBit(interface, ENC28J60_REG_ECON1, ECON1_TXRTS);
   return NO_ERROR;
}