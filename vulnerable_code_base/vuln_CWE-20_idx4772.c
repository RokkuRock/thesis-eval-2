error_t rza1EthInit(NetInterface *interface)
{
   error_t error;
   TRACE_INFO("Initializing RZ/A1 Ethernet MAC...\r\n");
   nicDriverInterface = interface;
   CPG.STBCR7 &= ~CPG_STBCR7_MSTP74;
   rza1EthInitGpio(interface);
   ETHER.ARSTR = ETHER_ARSTR_ARST;
   sleep(10);
   ETHER.EDSR0 = ETHER_EDSR0_ENT | ETHER_EDSR0_ENR;
   ETHER.EDMR0 = ETHER_EDMR0_SWRT | ETHER_EDMR0_SWRR;
   while(ETHER.EDMR0 & (ETHER_EDMR0_SWRT | ETHER_EDMR0_SWRR))
   {
   }
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
   rza1EthInitDmaDesc(interface);
   ETHER.EDMR0 = ETHER_EDMR0_DE | ETHER_EDMR0_DL_16;
   ETHER.TRSCER0 = 0;
   ETHER.TFTR0 = 0;
   ETHER.FDR0 = ETHER_FDR0_TFD_2048 | ETHER_FDR0_RFD_2048;
   ETHER.RMCR0 = ETHER_RMCR0_RNC;
   ETHER.RPADIR0 = 0;
   ETHER.FCFTR0 = ETHER_FCFTR0_RFF_8 | ETHER_FCFTR0_RFD_2048;
   ETHER.CSMR = 0;
   ETHER.ECMR0 |= ETH_ECMR0_MCT;
   ETHER.MAHR0 = (interface->macAddr.b[0] << 24) | (interface->macAddr.b[1] << 16) |
      (interface->macAddr.b[2] << 8) | interface->macAddr.b[3];
   ETHER.MALR0 = (interface->macAddr.b[4] << 8) | interface->macAddr.b[5];
   ETHER.TSU_TEN = 0;
   ETHER.RFLR0 = RZA1_ETH_RX_BUFFER_SIZE;
   ETHER.APR0 = 0;
   ETHER.MPR0 = 0;
   ETHER.TPAUSER0 = 0;
   ETHER.ECSIPR0 = 0;
   ETHER.EESIPR0 =  ETHER_EESIPR0_TWBIP | ETHER_EESIPR0_FRIP;
   R_INTC_Regist_Int_Func(INTC_ID_ETHERI, rza1EthIrqHandler);
   R_INTC_Set_Priority(INTC_ID_ETHERI, RZA1_ETH_IRQ_PRIORITY);
   ETHER.ECMR0 |= ETH_ECMR0_RE | ETH_ECMR0_TE;
   ETHER.EDRRR0 = ETHER_EDRRR0_RR;
   osSetEvent(&interface->nicTxEvent);
   return NO_ERROR;
}