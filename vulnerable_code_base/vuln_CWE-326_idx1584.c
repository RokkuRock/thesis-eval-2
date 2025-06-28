void flash_option_bytes_init(int boot_from_dfu)
{
    uint32_t val = 0xfffff8aa;
    if (boot_from_dfu){
        val &= ~(1<<27);  
    }
    else {
        if (solo_is_locked())
        {
            val = 0xfffff8cc;
        }
    }
    val &= ~(1<<26);  
    val &= ~(1<<25);  
    val &= ~(1<<24);  
    if (FLASH->OPTR == val)
    {
        return;
    }
    __disable_irq();
    while (FLASH->SR & (1<<16))
        ;
    flash_unlock();
    if (FLASH->CR & (1<<30))
    {
        FLASH->OPTKEYR = 0x08192A3B;
        FLASH->OPTKEYR = 0x4C5D6E7F;
    }
    FLASH->OPTR =val;
    FLASH->CR |= (1<<17);
    while (FLASH->SR & (1<<16))
        ;
    flash_lock();
    __enable_irq();
}