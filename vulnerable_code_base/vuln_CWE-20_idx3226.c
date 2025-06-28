error_t tja1100Init(NetInterface *interface)
{
   uint16_t value;
   TRACE_INFO("Initializing TJA1100...\r\n");
   if(interface->phyAddr >= 32)
   {
      interface->phyAddr = TJA1100_PHY_ADDR;
   }
   if(interface->smiDriver != NULL)
   {
      interface->smiDriver->init();
   }
   if(interface->extIntDriver != NULL)
   {
      interface->extIntDriver->init();
   }
   tja1100WritePhyReg(interface, TJA1100_BASIC_CTRL,
      TJA1100_BASIC_CTRL_RESET);
   while(tja1100ReadPhyReg(interface, TJA1100_BASIC_CTRL) &
      TJA1100_BASIC_CTRL_RESET)
   {
   }
   tja1100DumpPhyReg(interface);
   value = tja1100ReadPhyReg(interface, TJA1100_EXTENDED_CTRL);
   value |= TJA1100_EXTENDED_CTRL_CONFIG_EN;
   tja1100WritePhyReg(interface, TJA1100_EXTENDED_CTRL, value);
   value = tja1100ReadPhyReg(interface, TJA1100_CONFIG1);
   value &= ~TJA1100_CONFIG1_MII_MODE;
   value |= TJA1100_CONFIG1_MII_MODE_RMII_25MHZ;
   tja1100WritePhyReg(interface, TJA1100_CONFIG1, value);
   value = tja1100ReadPhyReg(interface, TJA1100_CONFIG1);
   value |= TJA1100_CONFIG1_AUTO_OP;
   tja1100WritePhyReg(interface, TJA1100_CONFIG1, value);
   interface->phyEvent = TRUE;
   osSetEvent(&netEvent);
   return NO_ERROR;
}