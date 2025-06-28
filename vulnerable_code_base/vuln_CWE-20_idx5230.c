void enc624j600UpdateMacConfig(NetInterface *interface)
{
   uint16_t duplexMode;
   duplexMode = enc624j600ReadReg(interface, ENC624J600_REG_ESTAT) & ESTAT_PHYDPX;
   if(duplexMode)
   {
      enc624j600WriteReg(interface, ENC624J600_REG_MACON2, MACON2_DEFER |
         MACON2_PADCFG2 | MACON2_PADCFG0 | MACON2_TXCRCEN | MACON2_R1 | MACON2_FULDPX);
      enc624j600WriteReg(interface, ENC624J600_REG_MABBIPG, 0x15);
   }
   else
   {
      enc624j600WriteReg(interface, ENC624J600_REG_MACON2, MACON2_DEFER |
         MACON2_PADCFG2 | MACON2_PADCFG0 | MACON2_TXCRCEN | MACON2_R1);
      enc624j600WriteReg(interface, ENC624J600_REG_MABBIPG, 0x12);
   }
}