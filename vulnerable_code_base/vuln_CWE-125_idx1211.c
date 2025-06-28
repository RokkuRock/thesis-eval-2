snmp_ber_decode_type(unsigned char *buff, uint32_t *buff_len, uint8_t *type)
{
  if(*buff_len == 0) {
    return NULL;
  }
  *type = *buff++;
  (*buff_len)--;
  return buff;
}