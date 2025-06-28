trustedDecryptDkgSecretAES(int *errStatus, char *errString, uint8_t *encrypted_dkg_secret,
                           uint32_t enc_len,
                           uint8_t *decrypted_dkg_secret) {
    LOG_INFO(__FUNCTION__);
    INIT_ERROR_STATE
    CHECK_STATE(encrypted_dkg_secret);
    CHECK_STATE(decrypted_dkg_secret);
    int status = AES_decrypt(encrypted_dkg_secret, enc_len, (char *) decrypted_dkg_secret,
                             3072);
    CHECK_STATUS2("aes decrypt data - encrypted_dkg_secret failed with status %d")
    SET_SUCCESS
    clean:
    ;
    LOG_INFO(__FUNCTION__ );
    LOG_INFO("SGX call completed");
}