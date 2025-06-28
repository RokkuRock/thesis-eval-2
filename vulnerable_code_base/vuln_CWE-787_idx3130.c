void trustedGenerateEcdsaKeyAES(int *errStatus, char *errString,
                                uint8_t *encryptedPrivateKey, uint32_t *enc_len, char *pub_key_x, char *pub_key_y) {
    LOG_INFO(__FUNCTION__);
    INIT_ERROR_STATE
    CHECK_STATE(encryptedPrivateKey);
    CHECK_STATE(pub_key_x);
    CHECK_STATE(pub_key_y);
    RANDOM_CHAR_BUF(rand_char, 32);
    mpz_t seed;
    mpz_init(seed);
    mpz_t skey;
    mpz_init(skey);
    point Pkey = point_init();
    mpz_import(seed, 32, 1, sizeof(rand_char[0]), 0, 0, rand_char);
    mpz_mod(skey, seed, curve->p);
    signature_extract_public_key(Pkey, skey, curve);
    SAFE_CHAR_BUF(arr_x, BUF_LEN);
    mpz_get_str(arr_x, ECDSA_SKEY_BASE, Pkey->x);
    int n_zeroes = 64 - strlen(arr_x);
    for (int i = 0; i < n_zeroes; i++) {
        pub_key_x[i] = '0';
    }
    strncpy(pub_key_x + n_zeroes, arr_x, 1024 - n_zeroes);
    SAFE_CHAR_BUF(arr_y, BUF_LEN);
    mpz_get_str(arr_y, ECDSA_SKEY_BASE, Pkey->y);
    n_zeroes = 64 - strlen(arr_y);
    for (int i = 0; i < n_zeroes; i++) {
        pub_key_y[i] = '0';
    }
    strncpy(pub_key_y + n_zeroes, arr_y, 1024 - n_zeroes);
    SAFE_CHAR_BUF(skey_str, ECDSA_SKEY_LEN);SAFE_CHAR_BUF(arr_skey_str, mpz_sizeinbase(skey, ECDSA_SKEY_BASE) + 2);
    mpz_get_str(arr_skey_str, ECDSA_SKEY_BASE, skey);
    n_zeroes = 64 - strlen(arr_skey_str);
    for (int i = 0; i < n_zeroes; i++) {
        skey_str[i] = '0';
    }
    strncpy(skey_str + n_zeroes, arr_skey_str, 65 - n_zeroes);
    skey_str[ECDSA_SKEY_LEN - 1] = 0;
    snprintf(errString, BUF_LEN, "skey len is %d\n", (int) strlen(skey_str));
    int status = AES_encrypt((char *) skey_str, encryptedPrivateKey, BUF_LEN);
    CHECK_STATUS("ecdsa private key encryption failed");
    *enc_len = strlen(skey_str) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
    status = AES_decrypt(encryptedPrivateKey, *enc_len, skey_str, ECDSA_SKEY_LEN);
    CHECK_STATUS2("ecdsa private key decr failed with status %d");
    SET_SUCCESS
    clean:
    mpz_clear(seed);
    mpz_clear(skey);
    point_clear(Pkey);
    LOG_INFO(__FUNCTION__ );
    LOG_INFO("SGX call completed");
}