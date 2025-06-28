snmp_engine_get_bulk(snmp_header_t *header, snmp_varbind_t *varbinds, uint32_t *varbinds_length)
{
  snmp_mib_resource_t *resource;
  uint32_t i, j, original_varbinds_length;
  uint32_t oid[SNMP_MAX_NR_VALUES][SNMP_MSG_OID_MAX_LEN];
  uint8_t repeater;
  original_varbinds_length = *varbinds_length;
  for(i = 0; i < original_varbinds_length; i++) {
    snmp_oid_copy(oid[i], varbinds[i].oid);
  }
  *varbinds_length = 0;
  for(i = 0; i < original_varbinds_length; i++) {
    if(i >= header->error_status_non_repeaters.non_repeaters) {
      break;
    }
    resource = snmp_mib_find_next(oid[i]);
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
      if(*varbinds_length < SNMP_MAX_NR_VALUES) {
        resource->handler(&varbinds[*varbinds_length], resource->oid);
        (*varbinds_length)++;
      }
    }
  }
  for(i = 0; i < header->error_index_max_repetitions.max_repetitions; i++) {
    repeater = 0;
    for(j = header->error_status_non_repeaters.non_repeaters; j < original_varbinds_length; j++) {
      resource = snmp_mib_find_next(oid[j]);
      if(!resource) {
        switch(header->version) {
        case SNMP_VERSION_1:
          header->error_status_non_repeaters.error_status = SNMP_STATUS_NO_SUCH_NAME;
          header->error_index_max_repetitions.error_index = *varbinds_length + 1;
          break;
        case SNMP_VERSION_2C:
          if(*varbinds_length < SNMP_MAX_NR_VALUES) {
            (&varbinds[*varbinds_length])->value_type = SNMP_DATA_TYPE_END_OF_MIB_VIEW;
            snmp_oid_copy((&varbinds[*varbinds_length])->oid, oid[j]);
            (*varbinds_length)++;
          }
          break;
        default:
          header->error_status_non_repeaters.error_status = SNMP_STATUS_NO_SUCH_NAME;
          header->error_index_max_repetitions.error_index = 0;
        }
      } else {
        if(*varbinds_length < SNMP_MAX_NR_VALUES) {
          resource->handler(&varbinds[*varbinds_length], resource->oid);
          (*varbinds_length)++;
          snmp_oid_copy(oid[j], resource->oid);
          repeater++;
        }
      }
    }
    if(repeater == 0) {
      break;
    }
  }
  return 0;
}