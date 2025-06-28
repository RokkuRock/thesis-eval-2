snmp_ber_encode_type(unsigned char *out, uint32_t *out_len, uint8_t type)
{
  *out-- = type;
  (*out_len)++;
  return out;
}