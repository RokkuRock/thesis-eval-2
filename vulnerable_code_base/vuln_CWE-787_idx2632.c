void trustedGetPublicEcdsaKeyAES(int *errStatus, char *errString,
                                 uint8_t *encryptedPrivateKey, uint32_t enc_len, char *pub_key_x, char *pub_key_y) {
    LOG_DEBUG(__FUNCTION__);
    INIT_ERROR_STATE
    SAFE_CHAR_BUF(skey, ECDSA_SKEY_LEN);
    mpz_t privateKeyMpz;
    mpz_init(privateKeyMpz);
    point pKey = point_init();
    point pKey_test = point_init();
    CHECK_STATE(encryptedPrivateKey);
    CHECK_STATE(pub_key_x);
    CHECK_STATE(pub_key_y);
    int status = AES_decrypt(encryptedPrivateKey, enc_len, skey, ECDSA_SKEY_LEN);
    CHECK_STATUS2("AES_decrypt failed with status %d");
    skey[enc_len - SGX_AESGCM_MAC_SIZE - SGX_AESGCM_IV_SIZE] = '\0';
    strncpy(errString, skey, 1024);
    status = mpz_set_str(privateKeyMpz, skey, ECDSA_SKEY_BASE);
    CHECK_STATUS("mpz_set_str failed for private key");
    signature_extract_public_key(pKey, privateKeyMpz, curve);
    point_multiplication(pKey_test, privateKeyMpz, curve->G, curve);
    if (!point_cmp(pKey, pKey_test)) {
        snprintf(errString, BUF_LEN, "Points are not equal");
        LOG_ERROR(errString);
        *errStatus = -11;
        goto clean;
    }
    SAFE_CHAR_BUF(arr_x, BUF_LEN);
    mpz_get_str(arr_x, ECDSA_SKEY_BASE, pKey->x);
    int n_zeroes = 64 - strlen(arr_x);
    for (int i = 0; i < n_zeroes; i++) {
        pub_key_x[i] = '0';
    }
    strncpy(pub_key_x + n_zeroes, arr_x, 1024 - n_zeroes);
    SAFE_CHAR_BUF(arr_y, BUF_LEN);
    mpz_get_str(arr_y, ECDSA_SKEY_BASE, pKey->y);
    n_zeroes = 64 - strlen(arr_y);
    for (int i = 0; i < n_zeroes; i++) {
        pub_key_y[i] = '0';
    }
    strncpy(pub_key_y + n_zeroes, arr_y, 1024 - n_zeroes);
    SET_SUCCESS
    clean:
    mpz_clear(privateKeyMpz);
    point_clear(pKey);
    point_clear(pKey_test);
    static uint64_t counter = 0;
    if (counter % 1000 == 0) {
        LOG_INFO(__FUNCTION__);
        LOG_INFO("Thousand SGX calls completed");
    }
    counter++;
}