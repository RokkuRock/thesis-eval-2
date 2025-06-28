void lpc546xxEthEventHandler(NetInterface *interface)
{
   error_t error;
   if((ENET->DMA_CH[0].DMA_CHX_STAT & ENET_DMA_CH_DMA_CHX_STAT_RI_MASK) != 0)
   {
      ENET->DMA_CH[0].DMA_CHX_STAT = ENET_DMA_CH_DMA_CHX_STAT_RI_MASK;
      do
      {
         error = lpc546xxEthReceivePacket(interface);
      } while(error != ERROR_BUFFER_EMPTY);
   }
   ENET->DMA_CH[0].DMA_CHX_INT_EN = ENET_DMA_CH_DMA_CHX_INT_EN_NIE_MASK |
      ENET_DMA_CH_DMA_CHX_INT_EN_RIE_MASK | ENET_DMA_CH_DMA_CHX_INT_EN_TIE_MASK;
}