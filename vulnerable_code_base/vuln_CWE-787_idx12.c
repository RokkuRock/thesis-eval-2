void trustedGetPublicSharesAES(int *errStatus, char *errString, uint8_t *encrypted_dkg_secret, uint32_t enc_len,
                               char *public_shares,
                               unsigned _t, unsigned _n) {
    LOG_INFO(__FUNCTION__);
    INIT_ERROR_STATE
    CHECK_STATE(encrypted_dkg_secret);
    CHECK_STATE(public_shares);
    CHECK_STATE(_t <= _n && _n > 0)
    SAFE_CHAR_BUF(decrypted_dkg_secret, DKG_MAX_SEALED_LEN);
    int status = AES_decrypt(encrypted_dkg_secret, enc_len, decrypted_dkg_secret,
                             DKG_MAX_SEALED_LEN);
    CHECK_STATUS2("aes decrypt data - encrypted_dkg_secret failed with status %d");
    status = calc_public_shares(decrypted_dkg_secret, public_shares, _t) != 0;
    CHECK_STATUS("t does not match polynomial in db");
    SET_SUCCESS
    clean:
    ;
    LOG_INFO("SGX call completed");
}