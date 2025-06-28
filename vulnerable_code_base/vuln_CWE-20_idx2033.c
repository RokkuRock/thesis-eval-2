error_t lpc546xxEthReceivePacket(NetInterface *interface)
{
   error_t error;
   size_t n;
   NetRxAncillary ancillary;
   if((rxDmaDesc[rxIndex].rdes3 & ENET_RDES3_OWN) == 0)
   {
      if((rxDmaDesc[rxIndex].rdes3 & ENET_RDES3_FD) != 0 &&
         (rxDmaDesc[rxIndex].rdes3 & ENET_RDES3_LD) != 0)
      {
         if((rxDmaDesc[rxIndex].rdes3 & ENET_RDES3_ES) == 0)
         {
            n = rxDmaDesc[rxIndex].rdes3 & ENET_RDES3_PL;
            n = MIN(n, LPC546XX_ETH_RX_BUFFER_SIZE);
            ancillary = NET_DEFAULT_RX_ANCILLARY;
            nicProcessPacket(interface, rxBuffer[rxIndex], n, &ancillary);
            error = NO_ERROR;
         }
         else
         {
            error = ERROR_INVALID_PACKET;
         }
      }
      else
      {
         error = ERROR_INVALID_PACKET;
      }
      rxDmaDesc[rxIndex].rdes0 = (uint32_t) rxBuffer[rxIndex];
      rxDmaDesc[rxIndex].rdes3 = ENET_RDES3_OWN | ENET_RDES3_IOC | ENET_RDES3_BUF1V;
      if(++rxIndex >= LPC546XX_ETH_RX_BUFFER_COUNT)
      {
         rxIndex = 0;
      }
   }
   else
   {
      error = ERROR_BUFFER_EMPTY;
   }
   ENET->DMA_CH[0].DMA_CHX_STAT = ENET_DMA_CH_DMA_CHX_STAT_RBU_MASK;
   ENET->DMA_CH[0].DMA_CHX_RXDESC_TAIL_PTR = 0;
   return error;
}