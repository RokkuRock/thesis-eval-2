snmp_ber_encode_length(unsigned char *out, uint32_t *out_len, uint8_t length)
{
  *out-- = length;
  (*out_len)++;
  return out;
}