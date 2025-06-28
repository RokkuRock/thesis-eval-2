sysName_handler(snmp_varbind_t *varbind, uint32_t *oid)
{
  snmp_api_set_string(varbind, oid, "Contiki-NG - "CONTIKI_TARGET_STRING);
}