error_t ksz8851UpdateMacAddrFilter(NetInterface *interface)
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
         crc = ksz8851CalcCrc(&entry->addr, sizeof(MacAddr));
         k = (crc >> 26) & 0x3F;
         hashTable[k / 16] |= (1 << (k % 16));
      }
   }
   ksz8851WriteReg(interface, KSZ8851_REG_MAHTR0, hashTable[0]);
   ksz8851WriteReg(interface, KSZ8851_REG_MAHTR1, hashTable[1]);
   ksz8851WriteReg(interface, KSZ8851_REG_MAHTR2, hashTable[2]);
   ksz8851WriteReg(interface, KSZ8851_REG_MAHTR3, hashTable[3]);
   TRACE_DEBUG("  MAHTR0 = %04" PRIX16 "\r\n", ksz8851ReadReg(interface, KSZ8851_REG_MAHTR0));
   TRACE_DEBUG("  MAHTR1 = %04" PRIX16 "\r\n", ksz8851ReadReg(interface, KSZ8851_REG_MAHTR1));
   TRACE_DEBUG("  MAHTR2 = %04" PRIX16 "\r\n", ksz8851ReadReg(interface, KSZ8851_REG_MAHTR2));
   TRACE_DEBUG("  MAHTR3 = %04" PRIX16 "\r\n", ksz8851ReadReg(interface, KSZ8851_REG_MAHTR3));
   return NO_ERROR;
}