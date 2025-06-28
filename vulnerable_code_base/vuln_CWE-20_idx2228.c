error_t ksz8851SendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t n;
   size_t length;
   Ksz8851TxHeader header;
   Ksz8851Context *context;
   context = (Ksz8851Context *) interface->nicContext;
   length = netBufferGetLength(buffer) - offset;
   if(length > ETH_MAX_FRAME_SIZE)
   {
      osSetEvent(&interface->nicTxEvent);
      return ERROR_INVALID_LENGTH;
   }
   n = ksz8851ReadReg(interface, KSZ8851_REG_TXMIR) & TXMIR_TXMA_MASK;
   if(n < (length + 8))
   {
      return ERROR_FAILURE;
   }
   netBufferRead(context->txBuffer, buffer, offset, length);
   header.controlWord = htole16(TX_CTRL_TXIC | (context->frameId++ & TX_CTRL_TXFID));
   header.byteCount = htole16(length);
   ksz8851SetBit(interface, KSZ8851_REG_RXQCR, RXQCR_SDA);
   ksz8851WriteFifo(interface, (uint8_t *) &header, sizeof(Ksz8851TxHeader));
   ksz8851WriteFifo(interface, context->txBuffer, length);
   ksz8851ClearBit(interface, KSZ8851_REG_RXQCR, RXQCR_SDA);
   ksz8851SetBit(interface, KSZ8851_REG_TXQCR, TXQCR_METFE);
   n = ksz8851ReadReg(interface, KSZ8851_REG_TXMIR) & TXMIR_TXMA_MASK;
   if(n >= (ETH_MAX_FRAME_SIZE + 8))
   {
      osSetEvent(&interface->nicTxEvent);
   }
   return NO_ERROR;
}