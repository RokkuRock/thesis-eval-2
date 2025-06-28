snmp_mib_find_next(uint32_t *oid)
{
  snmp_mib_resource_t *resource;
  resource = NULL;
  for(resource = list_head(snmp_mib);
      resource; resource = resource->next) {
    if(snmp_oid_cmp_oid(resource->oid, oid) > 0) {
      return resource;
    }
  }
  return NULL;
}