int AES_encrypt(char *message, uint8_t *encr_message, uint64_t encrLen) {
    if (!message) {
        LOG_ERROR("Null message in AES_encrypt");
        return -1;
    }
    if (!encr_message) {
        LOG_ERROR("Null encr message in AES_encrypt");
        return -2;
    }
    uint64_t len = strlen(message) + 1;
    if (len + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE > encrLen ) {
        LOG_ERROR("Output buffer too small");
        return -3;
    }
    sgx_read_rand(encr_message + SGX_AESGCM_MAC_SIZE, SGX_AESGCM_IV_SIZE);
    sgx_status_t status = sgx_rijndael128GCM_encrypt(&AES_key, (uint8_t*)message, strlen(message),
                                                     encr_message + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE,
                                                     encr_message + SGX_AESGCM_MAC_SIZE, SGX_AESGCM_IV_SIZE,
                                                     NULL, 0,
                                                     (sgx_aes_gcm_128bit_tag_t *) encr_message);
    return status;
}