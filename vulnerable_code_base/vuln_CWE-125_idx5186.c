snmp_api_set_oid(snmp_varbind_t *varbind, uint32_t *oid, uint32_t *ret_oid)
{
  snmp_api_replace_oid(varbind, oid);
  varbind->value_type = BER_DATA_TYPE_OID;
  varbind->value.oid = ret_oid;
}