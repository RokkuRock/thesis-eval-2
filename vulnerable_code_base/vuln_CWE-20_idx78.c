error_t ksz8851Init(NetInterface *interface)
{
   Ksz8851Context *context;
   context = (Ksz8851Context *) interface->nicContext;
   TRACE_INFO("Initializing KSZ8851 Ethernet controller...\r\n");
#if (KSZ8851_SPI_SUPPORT == ENABLED)
   interface->spiDriver->init();
#endif
   interface->extIntDriver->init();
   TRACE_DEBUG("CIDER=0x%04" PRIX16 "\r\n", ksz8851ReadReg(interface, KSZ8851_REG_CIDER));
   TRACE_DEBUG("PHY1ILR=0x%04" PRIX16 "\r\n", ksz8851ReadReg(interface, KSZ8851_REG_PHY1ILR));
   TRACE_DEBUG("PHY1IHR=0x%04" PRIX16 "\r\n", ksz8851ReadReg(interface, KSZ8851_REG_PHY1IHR));
   if(ksz8851ReadReg(interface, KSZ8851_REG_CIDER) != KSZ8851_REV_A3_ID)
   {
      return ERROR_WRONG_IDENTIFIER;
   }
   ksz8851DumpReg(interface);
   context->frameId = 0;
   context->txBuffer = memPoolAlloc(ETH_MAX_FRAME_SIZE);
   context->rxBuffer = memPoolAlloc(ETH_MAX_FRAME_SIZE);
   if(context->txBuffer == NULL || context->rxBuffer == NULL)
   {
      memPoolFree(context->txBuffer);
      memPoolFree(context->rxBuffer);
      return ERROR_OUT_OF_MEMORY;
   }
   ksz8851WriteReg(interface, KSZ8851_REG_MARH, htons(interface->macAddr.w[0]));
   ksz8851WriteReg(interface, KSZ8851_REG_MARM, htons(interface->macAddr.w[1]));
   ksz8851WriteReg(interface, KSZ8851_REG_MARL, htons(interface->macAddr.w[2]));
   ksz8851WriteReg(interface, KSZ8851_REG_TXCR, TXCR_TXFCE | TXCR_TXPE | TXCR_TXCE);
   ksz8851WriteReg(interface, KSZ8851_REG_TXFDPR, TXFDPR_TXFPAI);
   ksz8851WriteReg(interface, KSZ8851_REG_RXCR1,
      RXCR1_RXPAFMA | RXCR1_RXFCE | RXCR1_RXBE | RXCR1_RXME | RXCR1_RXUE);
   ksz8851WriteReg(interface, KSZ8851_REG_RXCR2,
      RXCR2_SRDBL2 | RXCR2_IUFFP | RXCR2_RXIUFCEZ);
   ksz8851WriteReg(interface, KSZ8851_REG_RXQCR, RXQCR_RXFCTE | RXQCR_ADRFE);
   ksz8851WriteReg(interface, KSZ8851_REG_RXFDPR, RXFDPR_RXFPAI);
   ksz8851WriteReg(interface, KSZ8851_REG_RXFCTR, 1);
   ksz8851ClearBit(interface, KSZ8851_REG_P1CR, P1CR_FORCE_DUPLEX);
   ksz8851SetBit(interface, KSZ8851_REG_P1CR, P1CR_RESTART_AN);
   ksz8851SetBit(interface, KSZ8851_REG_ISR, ISR_LCIS | ISR_TXIS |
      ISR_RXIS | ISR_RXOIS | ISR_TXPSIS | ISR_RXPSIS | ISR_TXSAIS |
      ISR_RXWFDIS | ISR_RXMPDIS | ISR_LDIS | ISR_EDIS | ISR_SPIBEIS);
   ksz8851SetBit(interface, KSZ8851_REG_IER, IER_LCIE | IER_TXIE | IER_RXIE);
   ksz8851SetBit(interface, KSZ8851_REG_TXCR, TXCR_TXE);
   ksz8851SetBit(interface, KSZ8851_REG_RXCR1, RXCR1_RXE);
   osSetEvent(&interface->nicTxEvent);
   interface->nicEvent = TRUE;
   osSetEvent(&netEvent);
   return NO_ERROR;
}