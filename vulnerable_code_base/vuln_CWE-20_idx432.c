error_t enc624j600SendPacket(NetInterface *interface,
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
   if(enc624j600ReadReg(interface, ENC624J600_REG_ECON1) & ECON1_TXRTS)
   {
      return ERROR_FAILURE;
   }
   enc624j600WriteReg(interface, ENC624J600_REG_EGPWRPT, ENC624J600_TX_BUFFER_START);
   enc624j600WriteBuffer(interface, ENC624J600_CMD_WGPDATA, buffer, offset);
   enc624j600WriteReg(interface, ENC624J600_REG_ETXST, ENC624J600_TX_BUFFER_START);
   enc624j600WriteReg(interface, ENC624J600_REG_ETXLEN, length);
   enc624j600ClearBit(interface, ENC624J600_REG_EIR, EIR_TXIF | EIR_TXABTIF);
   enc624j600SetBit(interface, ENC624J600_REG_ECON1, ECON1_TXRTS);
   return NO_ERROR;
}