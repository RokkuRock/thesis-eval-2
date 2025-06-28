error_t enc624j600SoftReset(NetInterface *interface)
{
   do
   {
      enc624j600WriteReg(interface, ENC624J600_REG_EUDAST, 0x1234);
   } while(enc624j600ReadReg(interface, ENC624J600_REG_EUDAST) != 0x1234);
   while((enc624j600ReadReg(interface, ENC624J600_REG_ESTAT) & ESTAT_CLKRDY) == 0)
   {
   }
   enc624j600SetBit(interface, ENC624J600_REG_ECON2, ECON2_ETHRST);
   sleep(1);
   if(enc624j600ReadReg(interface, ENC624J600_REG_EUDAST) != 0x0000)
   {
      return ERROR_FAILURE;
   }
   sleep(1);
   return NO_ERROR;
}