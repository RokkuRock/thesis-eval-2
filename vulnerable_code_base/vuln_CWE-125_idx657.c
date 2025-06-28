snmp_mib_find(uint32_t *oid)
{
  snmp_mib_resource_t *resource;
  resource = NULL;
  for(resource = list_head(snmp_mib);
      resource; resource = resource->next) {
    if(!snmp_oid_cmp_oid(oid, resource->oid)) {
      return resource;
    }
  }
  return NULL;
}