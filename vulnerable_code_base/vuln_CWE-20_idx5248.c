error_t enc28j60UpdateMacAddrFilter(NetInterface *interface)
{
   uint_t i;
   uint_t k;
   uint32_t crc;
   uint8_t hashTable[8];
   MacFilterEntry *entry;
   TRACE_DEBUG("Updating MAC filter...\r\n");
   osMemset(hashTable, 0, sizeof(hashTable));
   for(i = 0; i < MAC_ADDR_FILTER_SIZE; i++)
   {
      entry = &interface->macAddrFilter[i];
      if(entry->refCount > 0)
      {
         crc = enc28j60CalcCrc(&entry->addr, sizeof(MacAddr));
         k = (crc >> 23) & 0x3F;
         hashTable[k / 8] |= (1 << (k % 8));
      }
   }
   enc28j60WriteReg(interface, ENC28J60_REG_EHT0, hashTable[0]);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT1, hashTable[1]);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT2, hashTable[2]);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT3, hashTable[3]);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT4, hashTable[4]);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT5, hashTable[5]);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT6, hashTable[6]);
   enc28j60WriteReg(interface, ENC28J60_REG_EHT7, hashTable[7]);
   TRACE_DEBUG("  EHT0 = %02" PRIX8 "\r\n", enc28j60ReadReg(interface, ENC28J60_REG_EHT0));
   TRACE_DEBUG("  EHT1 = %02" PRIX8 "\r\n", enc28j60ReadReg(interface, ENC28J60_REG_EHT1));
   TRACE_DEBUG("  EHT2 = %02" PRIX8 "\r\n", enc28j60ReadReg(interface, ENC28J60_REG_EHT2));
   TRACE_DEBUG("  EHT3 = %02" PRIX8 "\r\n", enc28j60ReadReg(interface, ENC28J60_REG_EHT3));
   TRACE_DEBUG("  EHT0 = %02" PRIX8 "\r\n", enc28j60ReadReg(interface, ENC28J60_REG_EHT4));
   TRACE_DEBUG("  EHT1 = %02" PRIX8 "\r\n", enc28j60ReadReg(interface, ENC28J60_REG_EHT5));
   TRACE_DEBUG("  EHT2 = %02" PRIX8 "\r\n", enc28j60ReadReg(interface, ENC28J60_REG_EHT6));
   TRACE_DEBUG("  EHT3 = %02" PRIX8 "\r\n", enc28j60ReadReg(interface, ENC28J60_REG_EHT7));
   return NO_ERROR;
}