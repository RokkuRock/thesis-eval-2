trustedGetBlsPubKeyAES(int *errStatus, char *errString, uint8_t *encryptedPrivateKey, uint64_t key_len,
                       char *bls_pub_key) {
    LOG_DEBUG(__FUNCTION__);
    INIT_ERROR_STATE
    CHECK_STATE(bls_pub_key);
    CHECK_STATE(encryptedPrivateKey);
    SAFE_CHAR_BUF(skey_hex, ECDSA_SKEY_LEN);
    int status = AES_decrypt(encryptedPrivateKey, key_len, skey_hex, ECDSA_SKEY_LEN);
    CHECK_STATUS2("AES decrypt failed %d");
    skey_hex[ECDSA_SKEY_LEN - 1] = 0;
    status = calc_bls_public_key(skey_hex, bls_pub_key);
    CHECK_STATUS("could not calculate bls public key");
    SET_SUCCESS
    static uint64_t counter = 0;
    clean:
    if (counter % 1000 == 0) {
        LOG_INFO(__FUNCTION__);
        LOG_INFO("Thousand SGX calls completed");
    }
    counter++;
}