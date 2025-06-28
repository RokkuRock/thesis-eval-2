snmp_engine_get_next(snmp_header_t *header, snmp_varbind_t *varbinds, uint32_t varbinds_length)
{
  snmp_mib_resource_t *resource;
  uint32_t i;
  for(i = 0; i < varbinds_length; i++) {
    resource = snmp_mib_find_next(varbinds[i].oid);
    if(!resource) {
      switch(header->version) {
      case SNMP_VERSION_1:
        header->error_status_non_repeaters.error_status = SNMP_STATUS_NO_SUCH_NAME;
        header->error_index_max_repetitions.error_index = i + 1;
        break;
      case SNMP_VERSION_2C:
        (&varbinds[i])->value_type = SNMP_DATA_TYPE_END_OF_MIB_VIEW;
        break;
      default:
        header->error_status_non_repeaters.error_status = SNMP_STATUS_NO_SUCH_NAME;
        header->error_index_max_repetitions.error_index = 0;
      }
    } else {
      resource->handler(&varbinds[i], resource->oid);
    }
  }
  return 0;
}