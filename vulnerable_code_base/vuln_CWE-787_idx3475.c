void trustedCreateBlsKeyAES(int *errStatus, char *errString, const char *s_shares,
                            uint8_t *encryptedPrivateKey, uint64_t key_len, uint8_t *encr_bls_key,
                            uint32_t *enc_bls_key_len) {
    LOG_INFO(__FUNCTION__);
    INIT_ERROR_STATE
    CHECK_STATE(s_shares);
    CHECK_STATE(encryptedPrivateKey);
    CHECK_STATE(encr_bls_key);
    SAFE_CHAR_BUF(skey, ECDSA_SKEY_LEN);
    mpz_t sum;
    mpz_init(sum);
    mpz_set_ui(sum, 0);
    mpz_t q;
    mpz_init(q);
    mpz_set_str(q, "21888242871839275222246405745257275088548364400416034343698204186575808495617", 10);
    mpz_t bls_key;
    mpz_init(bls_key);
    int status = AES_decrypt(encryptedPrivateKey, key_len, skey, ECDSA_SKEY_LEN);
    CHECK_STATUS2("aes decrypt failed with status %d");
    skey[ECDSA_SKEY_LEN - 1] = 0;
    int num_shares = strlen(s_shares) / 192;
    for (int i = 0; i < num_shares; i++) { SAFE_CHAR_BUF(encr_sshare, 65);
        strncpy(encr_sshare, s_shares + 192 * i, 64);
        encr_sshare[64] = 0;
        SAFE_CHAR_BUF(s_share, 193);
        strncpy(s_share, s_shares + 192 * i, 192);
        s_share[192] = 0;
        SAFE_CHAR_BUF(common_key, 65);
        status = session_key_recover(skey, s_share, common_key);
        CHECK_STATUS("session_key_recover failed");
        common_key[64] = 0;
        SAFE_CHAR_BUF(decr_sshare, 65);
        status = xor_decrypt(common_key, encr_sshare, decr_sshare);
        CHECK_STATUS("xor_decrypt failed");
        decr_sshare[64] = 0;
        mpz_t decr_secret_share;
        mpz_init(decr_secret_share);
        if (mpz_set_str(decr_secret_share, decr_sshare, 16) == -1) {
            *errStatus = 111;
            snprintf(errString, BUF_LEN, "invalid decrypted secret share");
            LOG_ERROR(errString);
            mpz_clear(decr_secret_share);
            goto clean;
        }
        mpz_addmul_ui(sum, decr_secret_share, 1);
        mpz_clear(decr_secret_share);
    }
    mpz_mod(bls_key, sum, q);
    SAFE_CHAR_BUF(key_share, BLS_KEY_LENGTH);
    SAFE_CHAR_BUF(arr_skey_str, BUF_LEN);
    mpz_get_str(arr_skey_str, 16, bls_key);
    int n_zeroes = 64 - strlen(arr_skey_str);
    for (int i = 0; i < n_zeroes; i++) {
        key_share[i] = '0';
    }
    strncpy(key_share + n_zeroes, arr_skey_str, 65 - n_zeroes);
    key_share[BLS_KEY_LENGTH - 1] = 0;
    status = AES_encrypt(key_share, encr_bls_key, BUF_LEN);
    CHECK_STATUS2("aes encrypt bls private key failed with status %d ");
    *enc_bls_key_len = strlen(key_share) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
    SET_SUCCESS
    clean:
    mpz_clear(bls_key);
    mpz_clear(sum);
    mpz_clear(q);
    LOG_INFO(__FUNCTION__ );
    LOG_INFO("SGX call completed");
}