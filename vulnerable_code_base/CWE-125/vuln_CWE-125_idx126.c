snmp_ber_encode_null(unsigned char *out, uint32_t *out_len, uint8_t type)
{
  (*out_len)++;
  *out-- = 0x00;
  out = snmp_ber_encode_type(out, out_len, type);
  return out;
}