snmp_api_set_string(snmp_varbind_t *varbind, uint32_t *oid, char *string)
{
  snmp_api_replace_oid(varbind, oid);
  varbind->value_type = BER_DATA_TYPE_OCTET_STRING;
  varbind->value.string.string = string;
  varbind->value.string.length = strlen(string);
}