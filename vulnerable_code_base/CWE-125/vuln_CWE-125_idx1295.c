sysDescr_handler(snmp_varbind_t *varbind, uint32_t *oid)
{
  snmp_api_set_string(varbind, oid, CONTIKI_VERSION_STRING);
}