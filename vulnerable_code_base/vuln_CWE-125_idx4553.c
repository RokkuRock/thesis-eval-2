sysContact_handler(snmp_varbind_t *varbind, uint32_t *oid)
{
  snmp_api_set_string(varbind, oid, "Contiki-NG, https://github.com/contiki-ng/contiki-ng");
}