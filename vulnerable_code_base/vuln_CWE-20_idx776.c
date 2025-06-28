void enc624j600WritePhyReg(NetInterface *interface, uint8_t address,
   uint16_t data)
{
   enc624j600WriteReg(interface, ENC624J600_REG_MIREGADR, MIREGADR_R8 | address);
   enc624j600WriteReg(interface, ENC624J600_REG_MIWR, data);
   while((enc624j600ReadReg(interface, ENC624J600_REG_MISTAT) & MISTAT_BUSY) != 0)
   {
   }
}