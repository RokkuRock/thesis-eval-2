error_t dm9000UpdateMacAddrFilter(NetInterface *interface)
{
   uint_t i;
   uint_t k;
   uint32_t crc;
   uint8_t hashTable[8];
   MacFilterEntry *entry;
   TRACE_DEBUG("Updating MAC filter...\r\n");
   osMemset(hashTable, 0, sizeof(hashTable));
   hashTable[7] = 0x80;
   for(i = 0; i < MAC_ADDR_FILTER_SIZE; i++)
   {
      entry = &interface->macAddrFilter[i];
      if(entry->refCount > 0)
      {
         crc = dm9000CalcCrc(&entry->addr, sizeof(MacAddr));
         k = crc & 0x3F;
         hashTable[k / 8] |= (1 << (k % 8));
      }
   }
   for(i = 0; i < 8; i++)
   {
      dm9000WriteReg(DM9000_REG_MAR0 + i, hashTable[i]);
   }
   TRACE_DEBUG("  MAR = %02" PRIX8 " %02" PRIX8 " %02" PRIX8 " %02" PRIX8 " "
      "%02" PRIX8 " %02" PRIX8 " %02" PRIX8 " %02" PRIX8 "\r\n",
      dm9000ReadReg(DM9000_REG_MAR0), dm9000ReadReg(DM9000_REG_MAR1),
      dm9000ReadReg(DM9000_REG_MAR2), dm9000ReadReg(DM9000_REG_MAR3),
      dm9000ReadReg(DM9000_REG_MAR4), dm9000ReadReg(DM9000_REG_MAR5),
      dm9000ReadReg(DM9000_REG_MAR6), dm9000ReadReg(DM9000_REG_MAR7));
   return NO_ERROR;
}