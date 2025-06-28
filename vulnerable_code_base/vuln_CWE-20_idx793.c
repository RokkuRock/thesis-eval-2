uint16_t enc28j60ReadPhyReg(NetInterface *interface, uint16_t address)
{
   uint16_t data;
   enc28j60WriteReg(interface, ENC28J60_REG_MIREGADR, address & REG_ADDR_MASK);
   enc28j60WriteReg(interface, ENC28J60_REG_MICMD, MICMD_MIIRD);
   while((enc28j60ReadReg(interface, ENC28J60_REG_MISTAT) & MISTAT_BUSY) != 0)
   {
   }
   enc28j60WriteReg(interface, ENC28J60_REG_MICMD, 0);
   data = enc28j60ReadReg(interface, ENC28J60_REG_MIRDL);
   data |= enc28j60ReadReg(interface, ENC28J60_REG_MIRDH) << 8;
   return data;
}