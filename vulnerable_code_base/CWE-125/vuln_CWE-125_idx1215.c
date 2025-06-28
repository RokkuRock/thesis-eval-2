snmp_ber_decode_null(unsigned char *buf, uint32_t *buff_len)
{
  buf++;
  (*buff_len)--;
  buf++;
  (*buff_len)--;
  return buf;
}