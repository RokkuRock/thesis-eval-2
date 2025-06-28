uint8_t ethereum_extractThorchainData(const EthereumSignTx *msg,
                                          char *buffer) {
  uint16_t offset = 4 + (5 * 32);
  int16_t len = msg->data_length - offset;
  if (msg->has_data_length && len > 0) {
    memcpy(buffer, msg->data_initial_chunk.bytes + offset, len);
    return len < 256 ? (uint8_t)len : 0;
  }
  return 0;
}