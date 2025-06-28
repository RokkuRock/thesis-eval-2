sysServices_handler(snmp_varbind_t *varbind, uint32_t *oid)
{
  snmp_api_set_time_ticks(varbind, oid, clock_seconds() * 100);
}