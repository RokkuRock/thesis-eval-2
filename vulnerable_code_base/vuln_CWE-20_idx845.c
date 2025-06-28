error_t enc624j600UpdateMacAddrFilter(NetInterface *interface)
{
   uint_t i;
   uint_t k;
   uint32_t crc;
   uint16_t hashTable[4];
   MacFilterEntry *entry;
   TRACE_DEBUG("Updating MAC filter...\r\n");
   osMemset(hashTable, 0, sizeof(hashTable));
   for(i = 0; i < MAC_ADDR_FILTER_SIZE; i++)
   {
      entry = &interface->macAddrFilter[i];
      if(entry->refCount > 0)
      {
         crc = enc624j600CalcCrc(&entry->addr, sizeof(MacAddr));
         k = (crc >> 23) & 0x3F;
         hashTable[k / 16] |= (1 << (k % 16));
      }
   }
   enc624j600WriteReg(interface, ENC624J600_REG_EHT1, hashTable[0]);
   enc624j600WriteReg(interface, ENC624J600_REG_EHT2, hashTable[1]);
   enc624j600WriteReg(interface, ENC624J600_REG_EHT3, hashTable[2]);
   enc624j600WriteReg(interface, ENC624J600_REG_EHT4, hashTable[3]);
   TRACE_DEBUG("  EHT1 = %04" PRIX16 "\r\n", enc624j600ReadReg(interface, ENC624J600_REG_EHT1));
   TRACE_DEBUG("  EHT2 = %04" PRIX16 "\r\n", enc624j600ReadReg(interface, ENC624J600_REG_EHT2));
   TRACE_DEBUG("  EHT3 = %04" PRIX16 "\r\n", enc624j600ReadReg(interface, ENC624J600_REG_EHT3));
   TRACE_DEBUG("  EHT4 = %04" PRIX16 "\r\n", enc624j600ReadReg(interface, ENC624J600_REG_EHT4));
   return NO_ERROR;
}