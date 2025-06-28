void lpc546xxEthInitDmaDesc(NetInterface *interface)
{
   uint_t i;
   for(i = 0; i < LPC546XX_ETH_TX_BUFFER_COUNT; i++)
   {
      txDmaDesc[i].tdes0 = 0;
      txDmaDesc[i].tdes1 = 0;
      txDmaDesc[i].tdes2 = 0;
      txDmaDesc[i].tdes3 = 0;
   }
   txIndex = 0;
   for(i = 0; i < LPC546XX_ETH_RX_BUFFER_COUNT; i++)
   {
      rxDmaDesc[i].rdes0 = (uint32_t) rxBuffer[i];
      rxDmaDesc[i].rdes1 = 0;
      rxDmaDesc[i].rdes2 = 0;
      rxDmaDesc[i].rdes3 = ENET_RDES3_OWN | ENET_RDES3_IOC | ENET_RDES3_BUF1V;
   }
   rxIndex = 0;
   ENET->DMA_CH[0].DMA_CHX_TXDESC_LIST_ADDR = (uint32_t) &txDmaDesc[0];
   ENET->DMA_CH[0].DMA_CHX_TXDESC_RING_LENGTH = LPC546XX_ETH_TX_BUFFER_COUNT - 1;
   ENET->DMA_CH[0].DMA_CHX_RXDESC_LIST_ADDR = (uint32_t) &rxDmaDesc[0];
   ENET->DMA_CH[0].DMA_CHX_RXDESC_RING_LENGTH = LPC546XX_ETH_RX_BUFFER_COUNT - 1;
}