void svhandler_flash_pgm_word(void) {
  uint32_t dst = _param_1;
  uint32_t src = _param_2;
  if ((dst >= BSTRP_FLASH_SECT_START) &&
      (dst <= (BSTRP_FLASH_SECT_START + BSTRP_FLASH_SECT_LEN))) {
    return;
  }
  if ((dst >= BLDR_FLASH_SECT_START) &&
      (dst <= (BLDR_FLASH_SECT_START + 2 * BLDR_FLASH_SECT_LEN))) {
    return;
  }
  flash_clear_status_flags();
  flash_unlock();
  flash_program_word(dst, src);
  _param_1 = !!flash_chk_status();
  _param_2 = 0;
  _param_3 = 0;
  flash_wait_for_last_operation();
  FLASH_CR &= ~FLASH_CR_PG;
  FLASH_CR |= FLASH_CR_LOCK;
}