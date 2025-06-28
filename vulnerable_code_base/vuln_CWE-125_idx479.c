snmp_ber_decode_length(unsigned char *buff, uint32_t *buff_len, uint8_t *length)
{
  if(*buff_len == 0) {
    return NULL;
  }
  *length = *buff++;
  (*buff_len)--;
  return buff;
}