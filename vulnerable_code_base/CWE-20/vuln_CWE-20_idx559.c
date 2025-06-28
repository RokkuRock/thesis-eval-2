error_t tja1101Init(NetInterface *interface)
{
   uint16_t value;
   TRACE_INFO("Initializing TJA1101...\r\n");
   if(interface->phyAddr >= 32)
   {
      interface->phyAddr = TJA1101_PHY_ADDR;
   }
   if(interface->smiDriver != NULL)
   {
      interface->smiDriver->init();
   }
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->init();
   }
   tja1101WritePhyReg(interface, TJA1101_BASIC_CTRL,
      TJA1101_BASIC_CTRL_RESET);
   while(tja1101ReadPhyReg(interface, TJA1101_BASIC_CTRL) &
      TJA1101_BASIC_CTRL_RESET)
   {
   }
   tja1101DumpPhyReg(interface);
   value = tja1101ReadPhyReg(interface, TJA1101_EXTENDED_CTRL);
   value |= TJA1101_EXTENDED_CTRL_CONFIG_EN;
   tja1101WritePhyReg(interface, TJA1101_EXTENDED_CTRL, value);
   value = tja1101ReadPhyReg(interface, TJA1101_CONFIG1);
   value &= ~TJA1101_CONFIG1_MII_MODE;
   value |= TJA1101_CONFIG1_MII_MODE_RMII_25MHZ;
   tja1101WritePhyReg(interface, TJA1101_CONFIG1, value);
   value = tja1101ReadPhyReg(interface, TJA1101_COMM_CTRL);
   value |= TJA1101_COMM_CTRL_AUTO_OP;
   tja1101WritePhyReg(interface, TJA1101_COMM_CTRL, value);
   interface->phyEvent = TRUE;
   osSetEvent(&netEvent);
   return NO_ERROR;
}