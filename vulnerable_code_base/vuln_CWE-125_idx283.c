snmp_ber_encode_string_len(unsigned char *out, uint32_t *out_len, const char *str, uint32_t length)
{
  uint32_t i;
  str += length - 1;
  for(i = 0; i < length; ++i) {
    (*out_len)++;
    *out-- = (uint8_t)*str--;
  }
  out = snmp_ber_encode_length(out, out_len, length);
  out = snmp_ber_encode_type(out, out_len, BER_DATA_TYPE_OCTET_STRING);
  return out;
}