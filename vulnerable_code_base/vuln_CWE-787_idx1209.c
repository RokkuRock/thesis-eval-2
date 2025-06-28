void trustedEcdsaSignAES(int *errStatus, char *errString, uint8_t *encryptedPrivateKey, uint32_t enc_len,
                         const char *hash, char *sigR, char *sigS, uint8_t *sig_v, int base) {
    LOG_DEBUG(__FUNCTION__);
    INIT_ERROR_STATE
    CHECK_STATE(encryptedPrivateKey);
    CHECK_STATE(hash);
    CHECK_STATE(sigR);
    CHECK_STATE(sigS);
    SAFE_CHAR_BUF(skey, ECDSA_SKEY_LEN);
    mpz_t privateKeyMpz;
    mpz_init(privateKeyMpz);
    mpz_t msgMpz;
    mpz_init(msgMpz);
    signature sign = signature_init();
    int status = AES_decrypt(encryptedPrivateKey, enc_len, skey, ECDSA_SKEY_LEN);
    CHECK_STATUS2("aes decrypt failed with status %d");
    skey[enc_len - SGX_AESGCM_MAC_SIZE - SGX_AESGCM_IV_SIZE] = '\0';
    if (mpz_set_str(privateKeyMpz, skey, ECDSA_SKEY_BASE) == -1) {
        *errStatus = -1;
        snprintf(errString, BUF_LEN, "invalid secret key");
        LOG_ERROR(errString);
        goto clean;
    }
    if (mpz_set_str(msgMpz, hash, 16) == -1) {
        *errStatus = -1;
        snprintf(errString, BUF_LEN, "invalid message hash");
        LOG_ERROR(errString);
        goto clean;
    }
    signature_sign(sign, msgMpz, privateKeyMpz, curve);
    sigCounter++;
    if (sigCounter % 1000 == 0) {
        point Pkey = point_init();
        signature_extract_public_key(Pkey, privateKeyMpz, curve);
        if (!signature_verify(msgMpz, sign, Pkey, curve)) {
            *errStatus = -2;
            snprintf(errString, BUF_LEN, "signature is not verified! ");
            point_clear(Pkey);
            goto clean;
        }
        point_clear(Pkey);
    }
    SAFE_CHAR_BUF(arrM, BUF_LEN);
    mpz_get_str(arrM, 16, msgMpz);
    snprintf(errString, BUF_LEN, "message is %s ", arrM);
    SAFE_CHAR_BUF(arrR, BUF_LEN);
    mpz_get_str(arrR, base, sign->r);
    strncpy(sigR, arrR, 1024);
    SAFE_CHAR_BUF(arrS, BUF_LEN);
    mpz_get_str(arrS, base, sign->s);
    strncpy(sigS, arrS, 1024);
    *sig_v = sign->v;
    SET_SUCCESS
    clean:
    mpz_clear(privateKeyMpz);
    mpz_clear(msgMpz);
    signature_free(sign);
    LOG_DEBUG(__FUNCTION__ );
    LOG_DEBUG("SGX call completed");
}