trustedGenDkgSecretAES(int *errStatus, char *errString, uint8_t *encrypted_dkg_secret, uint32_t *enc_len, size_t _t) {
    LOG_INFO(__FUNCTION__);
    INIT_ERROR_STATE
    CHECK_STATE(encrypted_dkg_secret);
    SAFE_CHAR_BUF(dkg_secret, DKG_BUFER_LENGTH);
    int status = gen_dkg_poly(dkg_secret, _t);
    CHECK_STATUS("gen_dkg_poly failed")
    status = AES_encrypt(dkg_secret, encrypted_dkg_secret, 3 * BUF_LEN);
    CHECK_STATUS("SGX AES encrypt DKG poly failed");
    *enc_len = strlen(dkg_secret) + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE;
    SAFE_CHAR_BUF(decr_dkg_secret, DKG_BUFER_LENGTH);
    status = AES_decrypt(encrypted_dkg_secret, *enc_len, decr_dkg_secret,
                         DKG_BUFER_LENGTH);
    CHECK_STATUS("aes decrypt dkg poly failed");
    if (strcmp(dkg_secret, decr_dkg_secret) != 0) {
        snprintf(errString, BUF_LEN,
                 "encrypted poly is not equal to decrypted poly");
        LOG_ERROR(errString);
        *errStatus = -333;
        goto clean;
    }
    SET_SUCCESS
    clean:
    ;
    LOG_INFO(__FUNCTION__ );
    LOG_INFO("SGX call completed");
}