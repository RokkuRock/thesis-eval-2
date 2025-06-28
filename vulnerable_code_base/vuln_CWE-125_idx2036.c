snmp_ber_decode_integer(unsigned char *buf, uint32_t *buff_len, uint32_t *num)
{
  uint8_t i, len, type;
  buf = snmp_ber_decode_type(buf, buff_len, &type);
  if(buf == NULL || type != BER_DATA_TYPE_INTEGER) {
    return NULL;
  }
  buf = snmp_ber_decode_length(buf, buff_len, &len);
  if(buf == NULL || len > 4) {
    return NULL;
  }
  if(*buff_len < len) {
    return NULL;
  }
  *num = (uint32_t)(*buf++ & 0xFF);
  (*buff_len)--;
  for(i = 1; i < len; ++i) {
    *num <<= 8;
    *num |= (uint8_t)(*buf++ & 0xFF);
    (*buff_len)--;
  }
  return buf;
}