void svhandler_flash_pgm_blk(void) {
  uint32_t beginAddr = _param_1;
  uint32_t data = _param_2;
  uint32_t length = _param_3;
  if (beginAddr + length < beginAddr) return;
  if (((beginAddr >= BSTRP_FLASH_SECT_START) &&
       (beginAddr <= (BSTRP_FLASH_SECT_START + BSTRP_FLASH_SECT_LEN - 1))) ||
      (((beginAddr + length) >= BSTRP_FLASH_SECT_START) &&
       ((beginAddr + length) <=
        (BSTRP_FLASH_SECT_START + BSTRP_FLASH_SECT_LEN - 1)))) {
    return;
  }
  if (((beginAddr >= BLDR_FLASH_SECT_START) &&
       (beginAddr <= (BLDR_FLASH_SECT_START + 2 * BLDR_FLASH_SECT_LEN - 1))) ||
      (((beginAddr + length) >= BLDR_FLASH_SECT_START) &&
       ((beginAddr + length) <=
        (BLDR_FLASH_SECT_START + 2 * BLDR_FLASH_SECT_LEN - 1)))) {
    return;
  }
  flash_clear_status_flags();
  flash_unlock();
  flash_program(beginAddr, (uint8_t *)data, length);
  _param_1 = !!flash_chk_status();
  _param_2 = 0;
  _param_3 = 0;
  flash_wait_for_last_operation();
  FLASH_CR &= ~FLASH_CR_PG;
  FLASH_CR |= FLASH_CR_LOCK;
}