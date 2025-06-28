error_t am335xEthAddVlanAddrEntry(uint_t port, uint_t vlanId, MacAddr *macAddr)
{
   error_t error;
   uint_t index;
   Am335xAleEntry entry;
   index = am335xEthFindVlanAddrEntry(vlanId, macAddr);
   if(index >= CPSW_ALE_MAX_ENTRIES)
   {
      index = am335xEthFindFreeEntry();
   }
   if(index < CPSW_ALE_MAX_ENTRIES)
   {
      entry.word2 = 0;
      entry.word1 = CPSW_ALE_WORD1_ENTRY_TYPE_VLAN_ADDR;
      entry.word0 = 0;
      if(macIsMulticastAddr(macAddr))
      {
         entry.word2 |= CPSW_ALE_WORD2_SUPER |
            CPSW_ALE_WORD2_PORT_LIST(1 << port) |
            CPSW_ALE_WORD2_PORT_LIST(1 << CPSW_CH0);
         entry.word1 |= CPSW_ALE_WORD1_MCAST_FWD_STATE(0);
      }
      entry.word1 |= CPSW_ALE_WORD1_VLAN_ID(vlanId);
      entry.word1 |= (macAddr->b[0] << 8) | macAddr->b[1];
      entry.word0 |= (macAddr->b[2] << 24) | (macAddr->b[3] << 16) |
         (macAddr->b[4] << 8) | macAddr->b[5];
      am335xEthWriteEntry(index, &entry);
      error = NO_ERROR;
   }
   else
   {
      error = ERROR_FAILURE;
   }
   return error;
}