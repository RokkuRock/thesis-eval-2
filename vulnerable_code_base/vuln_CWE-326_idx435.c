void device_init()
{
    hw_init(LOW_FREQUENCY);
    if (! tsc_sensor_exists())
    {
        _NFC_status = nfc_init();
    }
    if (_NFC_status == NFC_IS_ACTIVE)
    {
        printf1(TAG_NFC, "Have NFC\r\n");
        isLowFreq = 1;
        IS_BUTTON_PRESSED = is_physical_button_pressed;
    }
    else
    {
        printf1(TAG_NFC, "Have NO NFC\r\n");
        hw_init(HIGH_FREQUENCY);
        isLowFreq = 0;
        device_init_button();
    }
    usbhid_init();
    ctaphid_init();
    ctap_init();
    device_migrate();
#if BOOT_TO_DFU
    flash_option_bytes_init(1);
#else
    flash_option_bytes_init(0);
#endif
}