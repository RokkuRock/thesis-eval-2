error_t am335xEthAddVlanEntry(uint_t port, uint_t vlanId)
{
   error_t error;
   uint_t index;
   Am335xAleEntry entry;
   index = am335xEthFindVlanEntry(vlanId);
   if(index >= CPSW_ALE_MAX_ENTRIES)
   {
      index = am335xEthFindFreeEntry();
   }
   if(index < CPSW_ALE_MAX_ENTRIES)
   {
      entry.word2 = 0;
      entry.word1 = CPSW_ALE_WORD1_ENTRY_TYPE_VLAN;
      entry.word0 = 0;
      entry.word1 |= CPSW_ALE_WORD1_VLAN_ID(vlanId);
      entry.word0 |= CPSW_ALE_WORD0_FORCE_UNTAG_EGRESS(1 << port) |
         CPSW_ALE_WORD0_FORCE_UNTAG_EGRESS(1 << CPSW_PORT0);
      entry.word0 |= CPSW_ALE_WORD0_VLAN_MEMBER_LIST(1 << port) |
         CPSW_ALE_WORD0_VLAN_MEMBER_LIST(1 << CPSW_PORT0);
      am335xEthWriteEntry(index, &entry);
      error = NO_ERROR;
   }
   else
   {
      error = ERROR_FAILURE;
   }
   return error;
}