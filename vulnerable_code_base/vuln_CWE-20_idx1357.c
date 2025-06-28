error_t enc624j600Init(NetInterface *interface)
{
   uint16_t temp;
   Enc624j600Context *context;
   TRACE_INFO("Initializing ENC624J600 Ethernet controller...\r\n");
   interface->spiDriver->init();
   interface->extIntDriver->init();
   context = (Enc624j600Context *) interface->nicContext;
   context->nextPacket = ENC624J600_RX_BUFFER_START;
   context->rxBuffer = memPoolAlloc(ETH_MAX_FRAME_SIZE);
   if(context->rxBuffer == NULL)
   {
      return ERROR_OUT_OF_MEMORY;
   }
   enc624j600SoftReset(interface);
   enc624j600WriteReg(interface, ENC624J600_REG_ECON2, ECON2_ETHEN | ECON2_STRCH);
   if(macCompAddr(&interface->macAddr, &MAC_UNSPECIFIED_ADDR))
   {
      temp = enc624j600ReadReg(interface, ENC624J600_REG_MAADR1);
      interface->macAddr.w[0] = letoh16(temp);
      temp = enc624j600ReadReg(interface, ENC624J600_REG_MAADR2);
      interface->macAddr.w[1] = letoh16(temp);
      temp = enc624j600ReadReg(interface, ENC624J600_REG_MAADR3);
      interface->macAddr.w[2] = letoh16(temp);
      macAddrToEui64(&interface->macAddr, &interface->eui64);
   }
   else
   {
      temp = htole16(interface->macAddr.w[0]);
      enc624j600WriteReg(interface, ENC624J600_REG_MAADR1, temp);
      temp = htole16(interface->macAddr.w[1]);
      enc624j600WriteReg(interface, ENC624J600_REG_MAADR2, temp);
      temp = htole16(interface->macAddr.w[2]);
      enc624j600WriteReg(interface, ENC624J600_REG_MAADR3, temp);
   }
   enc624j600WriteReg(interface, ENC624J600_REG_ERXST, ENC624J600_RX_BUFFER_START);
   enc624j600WriteReg(interface, ENC624J600_REG_ERXTAIL, ENC624J600_RX_BUFFER_STOP);
   enc624j600WriteReg(interface, ENC624J600_REG_ERXFCON, ERXFCON_HTEN |
      ERXFCON_CRCEN | ERXFCON_RUNTEN | ERXFCON_UCEN | ERXFCON_BCEN);
   enc624j600WriteReg(interface, ENC624J600_REG_EHT1, 0x0000);
   enc624j600WriteReg(interface, ENC624J600_REG_EHT2, 0x0000);
   enc624j600WriteReg(interface, ENC624J600_REG_EHT3, 0x0000);
   enc624j600WriteReg(interface, ENC624J600_REG_EHT4, 0x0000);
   enc624j600WriteReg(interface, ENC624J600_REG_MACON2,
      MACON2_DEFER | MACON2_PADCFG0 | MACON2_TXCRCEN | MACON2_R1);
   enc624j600WriteReg(interface, ENC624J600_REG_MAMXFL, ETH_MAX_FRAME_SIZE);
   enc624j600WritePhyReg(interface, ENC624J600_PHY_REG_PHANA, PHANA_ADPAUS0 |
      PHANA_AD100FD | PHANA_AD100 | PHANA_AD10FD | PHANA_AD10 | PHANA_ADIEEE0);
   enc624j600WriteReg(interface, ENC624J600_REG_EIR, 0x0000);
   enc624j600WriteReg(interface, ENC624J600_REG_EIE, EIE_INTIE |
      EIE_LINKIE | EIE_PKTIE | EIE_TXIE | EIE_TXABTIE);
   enc624j600SetBit(interface, ENC624J600_REG_ECON1, ECON1_RXEN);
   enc624j600DumpReg(interface);
   enc624j600DumpPhyReg(interface);
   osSetEvent(&interface->nicTxEvent);
   interface->nicEvent = TRUE;
   osSetEvent(&netEvent);
   return NO_ERROR;
}