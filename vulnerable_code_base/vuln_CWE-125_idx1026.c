snmp_mib_add(snmp_mib_resource_t *new_resource)
{
  snmp_mib_resource_t *resource;
  for(resource = list_head(snmp_mib);
      resource; resource = resource->next) {
    if(snmp_oid_cmp_oid(resource->oid, new_resource->oid) > 0) {
      break;
    }
  }
  if(resource == NULL) {
    list_add(snmp_mib, new_resource);
  } else {
    list_insert(snmp_mib, new_resource, resource);
  }
#if LOG_LEVEL == LOG_LEVEL_DBG
  LOG_DBG("Table after insert.\n");
  for(resource = list_head(snmp_mib);
      resource; resource = resource->next) {
    snmp_oid_print(resource->oid);
  }
#endif  
}