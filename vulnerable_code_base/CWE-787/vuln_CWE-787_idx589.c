void trustedGenerateSEK(int *errStatus, char *errString,
                        uint8_t *encrypted_sek, uint32_t *enc_len, char *sek_hex) {
    CALL_ONCE
    LOG_INFO(__FUNCTION__);
    INIT_ERROR_STATE
    CHECK_STATE(encrypted_sek);
    CHECK_STATE(sek_hex);
    RANDOM_CHAR_BUF(SEK_raw, SGX_AESGCM_KEY_SIZE);
    carray2Hex((uint8_t*) SEK_raw, SGX_AESGCM_KEY_SIZE, sek_hex);
    memcpy(AES_key, SEK_raw, SGX_AESGCM_KEY_SIZE);
    derive_DH_Key();
    sealHexSEK(errStatus, errString, encrypted_sek, enc_len, sek_hex);
    if (*errStatus != 0) {
        LOG_ERROR("sealHexSEK failed");
        goto clean;
    }
    SET_SUCCESS
    clean:
    LOG_INFO(__FUNCTION__ );
    LOG_INFO("SGX call completed");
}