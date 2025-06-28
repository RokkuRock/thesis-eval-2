error_t enc28j60Init(NetInterface *interface)
{
   uint8_t revisionId;
   Enc28j60Context *context;
   TRACE_INFO("Initializing ENC28J60 Ethernet controller...\r\n");
   interface->spiDriver->init();
   interface->extIntDriver->init();
   enc28j60SoftReset(interface);
   sleep(10);
   context = (Enc28j60Context *) interface->nicContext;
   context->currentBank = UINT16_MAX;
   context->nextPacket = ENC28J60_RX_BUFFER_START;
   context->rxBuffer = memPoolAlloc(ETH_MAX_FRAME_SIZE);
   if(context->rxBuffer == NULL)
   {
      return ERROR_OUT_OF_MEMORY;
   }
   revisionId = enc28j60ReadReg(interface, ENC28J60_REG_EREVID);
   TRACE_INFO("ENC28J60 revision ID: 0x%02X\r\n", revisionId);
   enc28j60WriteReg(interface, ENC28J60_REG_ECOCON, 0x00);
   enc28j60WriteReg(interface, ENC28J60_REG_MAADR1, interface->macAddr.b[0]);
   enc28j60WriteReg(interface, ENC28J60_REG_MAADR2, interface->macAddr.b[1]);
   enc28j60WriteReg(interface, ENC28J60_REG_MAADR3, interface->macAddr.b[2]);
   enc28j60WriteReg(interface, ENC28J60_REG_MAADR4, interface->macAddr.b[3]);
   enc28j60WriteReg(interface, ENC28J60_REG_MAADR5, interface->macAddr.b[4]);
   enc28j60WriteReg(interface, ENC28J60_REG_MAADR6, interface->macAddr.b[5]);
   enc28j60WriteReg(interface, ENC28J60_REG_ERXSTL, LSB(ENC28J60_RX_BUFFER_START));
   enc28j60WriteReg(interface, ENC28J60_REG_ERXSTH, MSB(ENC28J60_RX_BUFFER_START));
   enc28j60WriteReg(interface, ENC28J60_REG_ERXNDL, LSB(ENC28J60_RX_BUFFER_STOP));
   enc28j60WriteReg(interface, ENC28J60_REG_ERXNDH, MSB(ENC28J60_RX_BUFFER_STOP));
   enc28j60WriteReg(interface, ENC28J60_REG_ERXRDPTL, LSB(ENC28J60_RX_BUFFER_STOP));
   enc28j60WriteReg(interface, ENC28J60_REG_ERXRDPTH, MSB(ENC28J60_RX_BUFFER_STOP));
   enc28j60WriteReg(interface, ENC28J60_REG_ERXFCON, ERXFCON_UCEN |
      ERXFCON_CRCEN | ERXFCON_HTEN | ERXFCON_BCEN);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT0, 0x00);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT1, 0x00);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT2, 0x00);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT3, 0x00);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT4, 0x00);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT5, 0x00);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT6, 0x00);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT7, 0x00);
   enc28j60WriteReg(interface, ENC28J60_REG_MACON2, 0x00);
   enc28j60WriteReg(interface, ENC28J60_REG_MACON1,
      MACON1_TXPAUS | MACON1_RXPAUS | MACON1_MARXEN);
#if (ENC28J60_FULL_DUPLEX_SUPPORT == ENABLED)
   enc28j60WriteReg(interface, ENC28J60_REG_MACON3, MACON3_PADCFG(1) |
      MACON3_TXCRCEN | MACON3_FRMLNEN | MACON3_FULDPX);
#else
   enc28j60WriteReg(interface, ENC28J60_REG_MACON3, MACON3_PADCFG(1) |
      MACON3_TXCRCEN | MACON3_FRMLNEN);
#endif
   enc28j60WriteReg(interface, ENC28J60_REG_MACON4, MACON4_DEFER);
   enc28j60WriteReg(interface, ENC28J60_REG_MAMXFLL, LSB(ETH_MAX_FRAME_SIZE));
   enc28j60WriteReg(interface, ENC28J60_REG_MAMXFLH, MSB(ETH_MAX_FRAME_SIZE));
#if (ENC28J60_FULL_DUPLEX_SUPPORT == ENABLED)
   enc28j60WriteReg(interface, ENC28J60_REG_MABBIPG, 0x15);
#else
   enc28j60WriteReg(interface, ENC28J60_REG_MABBIPG, 0x12);
#endif
   enc28j60WriteReg(interface, ENC28J60_REG_MAIPGL, 0x12);
   enc28j60WriteReg(interface, ENC28J60_REG_MAIPGH, 0x0C);
   enc28j60WriteReg(interface, ENC28J60_REG_MACLCON2, 63);
#if (ENC28J60_FULL_DUPLEX_SUPPORT == ENABLED)
   enc28j60WritePhyReg(interface, ENC28J60_PHY_REG_PHCON1, PHCON1_PDPXMD);
#else
   enc28j60WritePhyReg(interface, ENC28J60_PHY_REG_PHCON1, 0x0000);
#endif
   enc28j60WritePhyReg(interface, ENC28J60_PHY_REG_PHCON2, PHCON2_HDLDIS);
   enc28j60WritePhyReg(interface, ENC28J60_PHY_REG_PHLCON,
      PHLCON_LACFG(4) | PHLCON_LBCFG(7) | PHLCON_LFRQ(0) | PHLCON_STRCH);
   enc28j60WriteReg(interface, ENC28J60_REG_EIR, 0x00);
   enc28j60WriteReg(interface, ENC28J60_REG_EIE, EIE_INTIE |
      EIE_PKTIE | EIE_LINKIE | EIE_TXIE | EIE_TXERIE);
   enc28j60WritePhyReg(interface, ENC28J60_PHY_REG_PHIE,
      PHIE_PLNKIE | PHIE_PGEIE);
   enc28j60SetBit(interface, ENC28J60_REG_ECON1, ECON1_RXEN);
   enc28j60DumpReg(interface);
   enc28j60DumpPhyReg(interface);
   osSetEvent(&interface->nicTxEvent);
   interface->nicEvent = TRUE;
   osSetEvent(&netEvent);
   return NO_ERROR;
}