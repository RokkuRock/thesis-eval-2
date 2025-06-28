snmp_ber_encode_unsigned_integer(unsigned char *out, uint32_t *out_len, uint8_t type, uint32_t number)
{
  uint32_t original_out_len;
  original_out_len = *out_len;
  do {
    (*out_len)++;
    *out-- = (uint8_t)(number & 0xFF);
    number >>= 8;
  } while(number);
  out = snmp_ber_encode_length(out, out_len, ((*out_len - original_out_len) & 0xFF));
  out = snmp_ber_encode_type(out, out_len, type);
  return out;
}