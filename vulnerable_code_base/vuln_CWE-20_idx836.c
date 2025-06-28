void svhandler_flash_erase_sector(void) {
  uint32_t sector = _param_1;
  if ((sector == FLASH_BOOTSTRAP_SECTOR) ||
      (sector >= FLASH_BOOT_SECTOR_FIRST && sector <= FLASH_BOOT_SECTOR_LAST)) {
    return;
  }
  flash_clear_status_flags();
  flash_unlock();
  flash_erase_sector(sector, FLASH_CR_PROGRAM_X32);
  _param_1 = !!flash_chk_status();
  _param_2 = 0;
  _param_3 = 0;
  flash_wait_for_last_operation();
  FLASH_CR &= ~FLASH_CR_PG;
  FLASH_CR |= FLASH_CR_LOCK;
}