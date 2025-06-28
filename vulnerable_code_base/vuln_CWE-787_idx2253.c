void trustedBlsSignMessageAES(int *errStatus, char *errString, uint8_t *encryptedPrivateKey,
                              uint32_t enc_len, char *_hashX,
                              char *_hashY, char *signature) {
    LOG_DEBUG(__FUNCTION__);
    INIT_ERROR_STATE
    CHECK_STATE(encryptedPrivateKey);
    CHECK_STATE(_hashX);
    CHECK_STATE(_hashY);
    CHECK_STATE(signature);
    SAFE_CHAR_BUF(key, BUF_LEN);SAFE_CHAR_BUF(sig, BUF_LEN);
    int status = AES_decrypt(encryptedPrivateKey, enc_len, key, BUF_LEN);
    CHECK_STATUS("AES decrypt failed")
    if (!enclave_sign(key, _hashX, _hashY, sig)) {
        strncpy(errString, "Enclave failed to create bls signature", BUF_LEN);
        LOG_ERROR(errString);
        *errStatus = -1;
        goto clean;
    }
    strncpy(signature, sig, BUF_LEN);
    if (strnlen(signature, BUF_LEN) < 10) {
        strncpy(errString, "Signature too short", BUF_LEN);
        LOG_ERROR(errString);
        *errStatus = -1;
        goto clean;
    }
    SET_SUCCESS
    LOG_DEBUG("SGX call completed");
    clean:
    ;
    LOG_DEBUG("SGX call completed");
}