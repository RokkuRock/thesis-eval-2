error_t dm9000Init(NetInterface *interface)
{
   uint_t i;
   uint16_t vendorId;
   uint16_t productId;
   uint8_t chipRevision;
   Dm9000Context *context;
   TRACE_INFO("Initializing DM9000 Ethernet controller...\r\n");
   interface->extIntDriver->init();
   context = (Dm9000Context *) interface->nicContext;
   context->queuedPackets = 0;
   context->txBuffer = memPoolAlloc(ETH_MAX_FRAME_SIZE);
   context->rxBuffer = memPoolAlloc(ETH_MAX_FRAME_SIZE);
   if(context->txBuffer == NULL || context->rxBuffer == NULL)
   {
      memPoolFree(context->txBuffer);
      memPoolFree(context->rxBuffer);
      return ERROR_OUT_OF_MEMORY;
   }
   vendorId = (dm9000ReadReg(DM9000_REG_VIDH) << 8) | dm9000ReadReg(DM9000_REG_VIDL);
   productId = (dm9000ReadReg(DM9000_REG_PIDH) << 8) | dm9000ReadReg(DM9000_REG_PIDL);
   chipRevision = dm9000ReadReg(DM9000_REG_CHIPR);
   if(vendorId != DM9000_VID || productId != DM9000_PID)
   {
      return ERROR_WRONG_IDENTIFIER;
   }
   if(chipRevision != DM9000A_CHIP_REV && chipRevision != DM9000B_CHIP_REV)
   {
      return ERROR_WRONG_IDENTIFIER;
   }
   dm9000WriteReg(DM9000_REG_GPR, 0x00);
   sleep(10);
   dm9000WriteReg(DM9000_REG_NCR, NCR_RST);
   while((dm9000ReadReg(DM9000_REG_NCR) & NCR_RST) != 0)
   {
   }
   dm9000WritePhyReg(DM9000_PHY_REG_BMCR, BMCR_RST);
   while((dm9000ReadPhyReg(DM9000_PHY_REG_BMCR) & BMCR_RST) != 0)
   {
   }
   TRACE_INFO("  VID = 0x%04" PRIX16 "\r\n", vendorId);
   TRACE_INFO("  PID = 0x%04" PRIX16 "\r\n", productId);
   TRACE_INFO("  CHIPR = 0x%02" PRIX8 "\r\n", chipRevision);
   TRACE_INFO("  PHYIDR1 = 0x%04" PRIX16 "\r\n", dm9000ReadPhyReg(DM9000_PHY_REG_PHYIDR1));
   TRACE_INFO("  PHYIDR2 = 0x%04" PRIX16 "\r\n", dm9000ReadPhyReg(DM9000_PHY_REG_PHYIDR2));
#if (DM9000_LOOPBACK_MODE == ENABLED)
   dm9000WriteReg(DM9000_REG_NCR, DM9000_LBK_PHY);
   dm9000WritePhyReg(DM9000_PHY_REG_BMCR, BMCR_LOOPBACK | BMCR_SPEED_SEL | BMCR_AN_EN | BMCR_DUPLEX_MODE);
#endif
   for(i = 0; i < 6; i++)
   {
      dm9000WriteReg(DM9000_REG_PAR0 + i, interface->macAddr.b[i]);
   }
   for(i = 0; i < 8; i++)
   {
      dm9000WriteReg(DM9000_REG_MAR0 + i, 0x00);
   }
   dm9000WriteReg(DM9000_REG_MAR7, 0x80);
   dm9000WriteReg(DM9000_REG_IMR, IMR_PAR);
   dm9000WriteReg(DM9000_REG_NSR, NSR_WAKEST | NSR_TX2END | NSR_TX1END);
   dm9000WriteReg(DM9000_REG_ISR, ISR_LNKCHG | ISR_UDRUN | ISR_ROO | ISR_ROS | ISR_PT | ISR_PR);
   dm9000WriteReg(DM9000_REG_IMR, IMR_PAR | IMR_LNKCHGI | IMR_PTI | IMR_PRI);
   dm9000WriteReg(DM9000_REG_RCR, RCR_DIS_LONG | RCR_DIS_CRC | RCR_RXEN);
   osSetEvent(&interface->nicTxEvent);
   interface->nicEvent = TRUE;
   osSetEvent(&netEvent);
   return NO_ERROR;
}