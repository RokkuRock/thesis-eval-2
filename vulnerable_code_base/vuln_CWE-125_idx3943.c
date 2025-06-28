snmp_api_set_time_ticks(snmp_varbind_t *varbind, uint32_t *oid, uint32_t integer)
{
  snmp_api_replace_oid(varbind, oid);
  varbind->value_type = SNMP_DATA_TYPE_TIME_TICKS;
  varbind->value.integer = integer;
}