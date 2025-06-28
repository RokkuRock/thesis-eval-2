error_t lpc546xxEthSendPacket(NetInterface *interface,
   const NetBuffer *buffer, size_t offset, NetTxAncillary *ancillary)
{
   size_t length;
   length = netBufferGetLength(buffer) - offset;
   if(length > LPC546XX_ETH_TX_BUFFER_SIZE)
   {
      osSetEvent(&interface->nicTxEvent);
      return ERROR_INVALID_LENGTH;
   }
   if((txDmaDesc[txIndex].tdes3 & ENET_TDES3_OWN) != 0)
   {
      return ERROR_FAILURE;
   }
   netBufferRead(txBuffer[txIndex], buffer, offset, length);
   txDmaDesc[txIndex].tdes0 = (uint32_t) txBuffer[txIndex];
   txDmaDesc[txIndex].tdes2 = ENET_TDES2_IOC | (length & ENET_TDES2_B1L);
   txDmaDesc[txIndex].tdes3 = ENET_TDES3_OWN | ENET_TDES3_FD | ENET_TDES3_LD;
   ENET->DMA_CH[0].DMA_CHX_STAT = ENET_DMA_CH_DMA_CHX_STAT_TBU_MASK;
   ENET->DMA_CH[0].DMA_CHX_TXDESC_TAIL_PTR = 0;
   if(++txIndex >= LPC546XX_ETH_TX_BUFFER_COUNT)
   {
      txIndex = 0;
   }
   if((txDmaDesc[txIndex].tdes3 & ENET_TDES3_OWN) == 0)
   {
      osSetEvent(&interface->nicTxEvent);
   }
   return NO_ERROR;
}