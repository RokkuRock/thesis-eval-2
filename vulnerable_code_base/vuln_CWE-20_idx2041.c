error_t am335xEthDeleteVlanAddrEntry(uint_t port, uint_t vlanId, MacAddr *macAddr)
{
   error_t error;
   uint_t index;
   Am335xAleEntry entry;
   index = am335xEthFindVlanAddrEntry(vlanId, macAddr);
   if(index < CPSW_ALE_MAX_ENTRIES)
   {
      entry.word2 = 0;
      entry.word1 = 0;
      entry.word0 = 0;
      am335xEthWriteEntry(index, &entry);
      error = NO_ERROR;
   }
   else
   {
      error = ERROR_NOT_FOUND;
   }
   return error;
}