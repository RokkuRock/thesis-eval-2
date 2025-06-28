uint16_t enc624j600ReadPhyReg(NetInterface *interface, uint8_t address)
{
   enc624j600WriteReg(interface, ENC624J600_REG_MIREGADR, MIREGADR_R8 | address);
   enc624j600WriteReg(interface, ENC624J600_REG_MICMD, MICMD_MIIRD);
   usleep(100);
   while((enc624j600ReadReg(interface, ENC624J600_REG_MISTAT) & MISTAT_BUSY) != 0)
   {
   }
   enc624j600WriteReg(interface, ENC624J600_REG_MICMD, 0x00);
   return enc624j600ReadReg(interface, ENC624J600_REG_MIRD);
}