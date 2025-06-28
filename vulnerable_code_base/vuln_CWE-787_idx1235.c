void trustedDecryptKeyAES(int *errStatus, char *errString, uint8_t *encryptedPrivateKey,
                          uint32_t enc_len, char *key) {
    LOG_DEBUG(__FUNCTION__);
    INIT_ERROR_STATE
    CHECK_STATE(encryptedPrivateKey);
    CHECK_STATE(key);
    *errStatus = -9;
    int status = AES_decrypt_DH(encryptedPrivateKey, enc_len, key, 3072);
    if (status != 0) {
        *errStatus = status;
        snprintf(errString, BUF_LEN, "aes decrypt failed with status %d", status);
        LOG_ERROR(errString);
        goto clean;
    }
    *errStatus = -10;
    uint64_t keyLen = strnlen(key, MAX_KEY_LENGTH);
    if (keyLen == MAX_KEY_LENGTH) {
        snprintf(errString, BUF_LEN, "Key is not null terminated");
        LOG_ERROR(errString);
        goto clean;
    }
    SET_SUCCESS
    clean:
    ;
}