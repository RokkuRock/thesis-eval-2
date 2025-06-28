snmp_engine(unsigned char *buff, uint32_t buff_len, unsigned char *out, uint32_t *out_len)
{
  static snmp_header_t header;
  static snmp_varbind_t varbinds[SNMP_MAX_NR_VALUES];
  static uint32_t varbind_length = SNMP_MAX_NR_VALUES;
  buff = snmp_message_decode(buff, buff_len, &header, varbinds, &varbind_length);
  if(buff == NULL) {
    return NULL;
  }
  if(header.version != SNMP_VERSION_1) {
    if(strncmp(header.community.community, SNMP_COMMUNITY, header.community.length)) {
      LOG_ERR("Request with invalid community\n");
      return NULL;
    }
  }
  switch(header.pdu_type) {
  case SNMP_DATA_TYPE_PDU_GET_REQUEST:
    if(snmp_engine_get(&header, varbinds, varbind_length) == -1) {
      return NULL;
    }
    break;
  case SNMP_DATA_TYPE_PDU_GET_NEXT_REQUEST:
    if(snmp_engine_get_next(&header, varbinds, varbind_length) == -1) {
      return NULL;
    }
    break;
  case SNMP_DATA_TYPE_PDU_GET_BULK:
    if(snmp_engine_get_bulk(&header, varbinds, &varbind_length) == -1) {
      return NULL;
    }
    break;
  default:
    LOG_ERR("Invalid request type");
    return NULL;
  }
  header.pdu_type = SNMP_DATA_TYPE_PDU_GET_RESPONSE;
  out = snmp_message_encode(out, out_len, &header, varbinds, varbind_length);
  return ++out;
}