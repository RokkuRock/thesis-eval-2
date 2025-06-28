void dm9000WritePhyReg(uint8_t address, uint16_t data)
{
   dm9000WriteReg(DM9000_REG_EPAR, 0x40 | address);
   dm9000WriteReg(DM9000_REG_EPDRL, LSB(data));
   dm9000WriteReg(DM9000_REG_EPDRH, MSB(data));
   dm9000WriteReg(DM9000_REG_EPCR, EPCR_EPOS | EPCR_ERPRW);
   while((dm9000ReadReg(DM9000_REG_EPCR) & EPCR_ERRE) != 0)
   {
   }
   usleep(5);
   dm9000WriteReg(DM9000_REG_EPCR, EPCR_EPOS);
}