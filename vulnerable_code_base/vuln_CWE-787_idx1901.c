void trustedDkgVerifyAES(int *errStatus, char *errString, const char *public_shares, const char *s_share,
                         uint8_t *encryptedPrivateKey, uint64_t enc_len, unsigned _t, int _ind, int *result) {
    LOG_INFO(__FUNCTION__);
    INIT_ERROR_STATE
    CHECK_STATE(public_shares);
    CHECK_STATE(s_share);
    CHECK_STATE(encryptedPrivateKey);
    SAFE_CHAR_BUF(skey, ECDSA_SKEY_LEN);
    mpz_t s;
    mpz_init(s);
    int status = AES_decrypt(encryptedPrivateKey, enc_len, skey, ECDSA_SKEY_LEN);
    CHECK_STATUS2("AES_decrypt failed (in trustedDkgVerifyAES) with status %d");
    SAFE_CHAR_BUF(encr_sshare, ECDSA_SKEY_LEN);
    strncpy(encr_sshare, s_share, ECDSA_SKEY_LEN - 1);
    SAFE_CHAR_BUF(common_key, ECDSA_SKEY_LEN);
    status = session_key_recover(skey, s_share, common_key);
    CHECK_STATUS("session_key_recover failed");
    SAFE_CHAR_BUF(decr_sshare, ECDSA_SKEY_LEN);
    status=xor_decrypt(common_key, encr_sshare, decr_sshare);
    CHECK_STATUS("xor_decrypt failed")
    status  = mpz_set_str(s, decr_sshare, 16);
    CHECK_STATUS("invalid decr secret share");
    *result = Verification(public_shares, s, _t, _ind);
    SET_SUCCESS
    clean:
    mpz_clear(s);
    LOG_INFO(__FUNCTION__ );
    LOG_INFO("SGX call completed");
}