void trustedSetEncryptedDkgPolyAES(int *errStatus, char *errString, uint8_t *encrypted_poly, uint32_t enc_len) {
    LOG_INFO(__FUNCTION__);
    INIT_ERROR_STATE
    CHECK_STATE(encrypted_poly);
    memset(getThreadLocalDecryptedDkgPoly(), 0, DKG_BUFER_LENGTH);
    int status = AES_decrypt(encrypted_poly, enc_len, (char *) getThreadLocalDecryptedDkgPoly(),
                             DKG_BUFER_LENGTH);
    CHECK_STATUS2("sgx_unseal_data - encrypted_poly failed with status %d")
    SET_SUCCESS
    clean:
    ;
    LOG_INFO(__FUNCTION__ );
    LOG_INFO("SGX call completed");
}