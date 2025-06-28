void trustedGetEncryptedSecretShareAES(int *errStatus, char *errString, uint8_t *encrypted_skey, uint32_t *dec_len,
                                       char *result_str, char *s_shareG2, char *pub_keyB, uint8_t _t, uint8_t _n,
                                       uint8_t ind) {
    LOG_INFO(__FUNCTION__);
    INIT_ERROR_STATE
    uint32_t enc_len;
    int status;
    CHECK_STATE(encrypted_skey);
    CHECK_STATE(result_str);
    CHECK_STATE(s_shareG2);
    CHECK_STATE(pub_keyB);
    LOG_DEBUG(__FUNCTION__);
    SAFE_CHAR_BUF(skey, ECDSA_SKEY_LEN);
    SAFE_CHAR_BUF(pub_key_x, BUF_LEN);SAFE_CHAR_BUF(pub_key_y, BUF_LEN);
    trustedGenerateEcdsaKeyAES(&status, errString, encrypted_skey, &enc_len, pub_key_x, pub_key_y);
    CHECK_STATUS("trustedGenerateEcdsaKeyAES failed");
    status = AES_decrypt(encrypted_skey, enc_len, skey, ECDSA_SKEY_LEN);
    skey[ECDSA_SKEY_LEN - 1] = 0;
    CHECK_STATUS2("AES_decrypt failed (in trustedGetEncryptedSecretShareAES) with status %d");
    *dec_len = enc_len;
    SAFE_CHAR_BUF(common_key, ECDSA_SKEY_LEN);
    status = gen_session_key(skey, pub_keyB, common_key);
    CHECK_STATUS("gen_session_key failed")
    SAFE_CHAR_BUF(s_share, ECDSA_SKEY_LEN);
    status = calc_secret_share(getThreadLocalDecryptedDkgPoly(), s_share, _t, _n, ind);
    CHECK_STATUS("calc secret share failed")
    status = calc_secret_shareG2(s_share, s_shareG2);
    CHECK_STATUS("invalid decr secret share");
    SAFE_CHAR_BUF(cypher, ECDSA_SKEY_LEN);
    status=xor_encrypt(common_key, s_share, cypher);
    CHECK_STATUS("xor_encrypt failed")
    strncpy(result_str, cypher, strlen(cypher));
    strncpy(result_str + strlen(cypher), pub_key_x, strlen(pub_key_x));
    strncpy(result_str + strlen(pub_key_x) + strlen(pub_key_y), pub_key_y, strlen(pub_key_y));
    SET_SUCCESS
    clean:
    ;
    LOG_INFO(__FUNCTION__ );
    LOG_INFO("SGX call completed");
}