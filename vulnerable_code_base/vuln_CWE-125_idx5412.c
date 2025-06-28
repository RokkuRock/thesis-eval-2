sysObjectID_handler(snmp_varbind_t *varbind, uint32_t *oid)
{
  OID(sysObjectID_oid, 1, 3, 6, 1, 4, 1, 54352);
  snmp_api_set_oid(varbind, oid, sysObjectID_oid);
}