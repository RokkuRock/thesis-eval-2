error_t lpc546xxEthInit(NetInterface *interface)
{
   error_t error;
   TRACE_INFO("Initializing LPC546xx Ethernet MAC...\r\n");
   nicDriverInterface = interface;
   CLOCK_EnableClock(kCLOCK_Eth);
   RESET_PeripheralReset(kETH_RST_SHIFT_RSTn);
   lpc546xxEthInitGpio(interface);
   ENET->DMA_MODE |= ENET_DMA_MODE_SWR_MASK;
   while((ENET->DMA_MODE & ENET_DMA_MODE_SWR_MASK) != 0)
   {
   }
   ENET->MAC_MDIO_ADDR = ENET_MAC_MDIO_ADDR_CR(4);
   if(interface->phyDriver != NULL)
   {
      error = interface->phyDriver->init(interface);
   }
   else if(interface->switchDriver != NULL)
   {
      error = interface->switchDriver->init(interface);
   }
   else
   {
      error = ERROR_FAILURE;
   }
   if(error)
   {
      return error;
   }
   ENET->MAC_CONFIG = ENET_MAC_CONFIG_PS_MASK | ENET_MAC_CONFIG_DO_MASK;
   ENET->MAC_ADDR_LOW = interface->macAddr.w[0] | (interface->macAddr.w[1] << 16);
   ENET->MAC_ADDR_HIGH = interface->macAddr.w[2];
   ENET->MAC_FRAME_FILTER = 0;
   ENET->MAC_TX_FLOW_CTRL_Q[0] = 0;
   ENET->MAC_RX_FLOW_CTRL = 0;
   ENET->MAC_RXQ_CTRL[0] = ENET_MAC_RXQ_CTRL_RXQ0EN(1);
   ENET->DMA_MODE = ENET_DMA_MODE_PR(0);
   ENET->DMA_SYSBUS_MODE |= ENET_DMA_SYSBUS_MODE_AAL_MASK;
   ENET->DMA_CH[0].DMA_CHX_CTRL = ENET_DMA_CH_DMA_CHX_CTRL_DSL(0);
   ENET->DMA_CH[0].DMA_CHX_TX_CTRL = ENET_DMA_CH_DMA_CHX_TX_CTRL_TxPBL(1);
   ENET->DMA_CH[0].DMA_CHX_RX_CTRL = ENET_DMA_CH_DMA_CHX_RX_CTRL_RxPBL(1) |
      ENET_DMA_CH_DMA_CHX_RX_CTRL_RBSZ(LPC546XX_ETH_RX_BUFFER_SIZE / 4);
   ENET->MTL_QUEUE[0].MTL_TXQX_OP_MODE |= ENET_MTL_QUEUE_MTL_TXQX_OP_MODE_TQS(7) |
      ENET_MTL_QUEUE_MTL_TXQX_OP_MODE_TXQEN(2) |
      ENET_MTL_QUEUE_MTL_TXQX_OP_MODE_TSF_MASK;
   ENET->MTL_QUEUE[0].MTL_RXQX_OP_MODE |= ENET_MTL_QUEUE_MTL_RXQX_OP_MODE_RQS(7) |
      ENET_MTL_QUEUE_MTL_RXQX_OP_MODE_RSF_MASK;
   lpc546xxEthInitDmaDesc(interface);
   ENET->MAC_INTR_EN = 0;
   ENET->DMA_CH[0].DMA_CHX_INT_EN = ENET_DMA_CH_DMA_CHX_INT_EN_NIE_MASK |
      ENET_DMA_CH_DMA_CHX_INT_EN_RIE_MASK | ENET_DMA_CH_DMA_CHX_INT_EN_TIE_MASK;
   NVIC_SetPriorityGrouping(LPC546XX_ETH_IRQ_PRIORITY_GROUPING);
   NVIC_SetPriority(ETHERNET_IRQn, NVIC_EncodePriority(LPC546XX_ETH_IRQ_PRIORITY_GROUPING,
      LPC546XX_ETH_IRQ_GROUP_PRIORITY, LPC546XX_ETH_IRQ_SUB_PRIORITY));
   ENET->MAC_CONFIG |= ENET_MAC_CONFIG_TE_MASK | ENET_MAC_CONFIG_RE_MASK;
   ENET->DMA_CH[0].DMA_CHX_TX_CTRL |= ENET_DMA_CH_DMA_CHX_TX_CTRL_ST_MASK;
   ENET->DMA_CH[0].DMA_CHX_RX_CTRL |= ENET_DMA_CH_DMA_CHX_RX_CTRL_SR_MASK;
   osSetEvent(&interface->nicTxEvent);
   return NO_ERROR;
}