snmp_api_replace_oid(snmp_varbind_t *varbind, uint32_t *oid)
{
  uint8_t i;
  i = 0;
  while(oid[i] != ((uint32_t)-1)) {
    varbind->oid[i] = oid[i];
    i++;
  }
  varbind->oid[i] = ((uint32_t)-1);
}