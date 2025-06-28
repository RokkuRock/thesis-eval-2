void sealHexSEK(int *errStatus, char *errString,
                        uint8_t *encrypted_sek, uint32_t *enc_len, char *sek_hex) {
    CALL_ONCE
    LOG_INFO(__FUNCTION__);
    INIT_ERROR_STATE
    CHECK_STATE(encrypted_sek);
    CHECK_STATE(sek_hex);
    CHECK_STATE(strnlen(sek_hex, 33) == 32)
    uint64_t plaintextLen = strlen(sek_hex) + 1;
    uint64_t sealedLen = sgx_calc_sealed_data_size(0, plaintextLen);
    sgx_attributes_t attribute_mask;
    attribute_mask.flags = 0xfffffffffffffff3;
    attribute_mask.xfrm = 0x0;
    sgx_misc_select_t misc = 0xF0000000;
    sgx_status_t status = sgx_seal_data_ex(SGX_KEYPOLICY_MRENCLAVE, attribute_mask, misc, 0, NULL, plaintextLen, (uint8_t *) sek_hex, sealedLen,
                                           (sgx_sealed_data_t *) encrypted_sek);
    CHECK_STATUS("seal SEK failed after SEK generation");
    uint32_t encrypt_text_length = sgx_get_encrypt_txt_len((const sgx_sealed_data_t *)encrypted_sek);
    CHECK_STATE(encrypt_text_length = plaintextLen);
    SAFE_CHAR_BUF(unsealedKey, BUF_LEN);
    uint32_t decLen = BUF_LEN;
    uint32_t add_text_length = sgx_get_add_mac_txt_len((const sgx_sealed_data_t *)encrypted_sek);
    CHECK_STATE(add_text_length == 0);
    CHECK_STATE(sgx_is_within_enclave(encrypted_sek,sizeof(sgx_sealed_data_t)));
    status = sgx_unseal_data((const sgx_sealed_data_t *)encrypted_sek, NULL, NULL,
                             (uint8_t *) unsealedKey, &decLen );
    CHECK_STATUS("seal/unseal SEK failed after SEK generation in unseal");
    *enc_len = sealedLen;
    SET_SUCCESS
    clean:
    LOG_INFO(__FUNCTION__ );
    LOG_INFO("SGX call completed");
}