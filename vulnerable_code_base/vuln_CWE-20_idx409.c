uint16_t dm9000ReadPhyReg(uint8_t address)
{
   dm9000WriteReg(DM9000_REG_EPAR, 0x40 | address);
   dm9000WriteReg(DM9000_REG_EPCR, EPCR_EPOS | EPCR_ERPRR);
   while((dm9000ReadReg(DM9000_REG_EPCR) & EPCR_ERRE) != 0)
   {
   }
   dm9000WriteReg(DM9000_REG_EPCR, EPCR_EPOS);
   usleep(5);
   return (dm9000ReadReg(DM9000_REG_EPDRH) << 8) | dm9000ReadReg(DM9000_REG_EPDRL);
}