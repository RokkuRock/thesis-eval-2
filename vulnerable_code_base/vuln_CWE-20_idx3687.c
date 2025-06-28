error_t dm9000SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t i;
   size_t length;
   uint16_t *p;
   Dm9000Context *context;
   context = (Dm9000Context *) interface->nicContext;
   length = netBufferGetLength(buffer) - offset;
   if(length > ETH_MAX_FRAME_SIZE)
   {
      osSetEvent(&interface->nicTxEvent);
      return ERROR_INVALID_LENGTH;
   }
   netBufferRead(context->txBuffer, buffer, offset, length);
   dm9000WriteReg(DM9000_REG_MWCMDX, 0);
   DM9000_INDEX_REG = DM9000_REG_MWCMD;
   p = (uint16_t *) context->txBuffer;
   for(i = length; i > 1; i -= 2)
   {
      DM9000_DATA_REG = *(p++);
   }
   if(i > 0)
   {
      DM9000_DATA_REG = *((uint8_t *) p);
   }
   dm9000WriteReg(DM9000_REG_TXPLL, LSB(length));
   dm9000WriteReg(DM9000_REG_TXPLH, MSB(length));
   dm9000WriteReg(DM9000_REG_ISR, ISR_PT);
   dm9000WriteReg(DM9000_REG_TCR, TCR_TXREQ);
   context->queuedPackets++;
   return NO_ERROR;
}