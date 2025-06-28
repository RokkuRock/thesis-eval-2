void enc28j60WritePhyReg(NetInterface *interface, uint16_t address,
   uint16_t data)
{
   enc28j60WriteReg(interface, ENC28J60_REG_MIREGADR, address & REG_ADDR_MASK);
   enc28j60WriteReg(interface, ENC28J60_REG_MIWRL, LSB(data));
   enc28j60WriteReg(interface, ENC28J60_REG_MIWRH, MSB(data));
   while((enc28j60ReadReg(interface, ENC28J60_REG_MISTAT) & MISTAT_BUSY) != 0)
   {
   }
}