int AES_decrypt_DH(uint8_t *encr_message, uint64_t length, char *message, uint64_t msgLen) {
    if (!message) {
        LOG_ERROR("Null message in AES_encrypt_DH");
        return -1;
    }
    if (!encr_message) {
        LOG_ERROR("Null encr message in AES_encrypt_DH");
        return -2;
    }
    if (length < SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE) {
        LOG_ERROR("length < SGX_AESGCM_MAC_SIZE - SGX_AESGCM_IV_SIZE");
        return -1;
    }
    uint64_t len = length - SGX_AESGCM_MAC_SIZE - SGX_AESGCM_IV_SIZE;
    if (msgLen < len) {
        LOG_ERROR("Output buffer not large enough");
        return -2;
    }
    sgx_status_t status = sgx_rijndael128GCM_decrypt(&AES_DH_key,
                                                     encr_message + SGX_AESGCM_MAC_SIZE + SGX_AESGCM_IV_SIZE, len,
                                                     (unsigned char*) message,
                                                     encr_message + SGX_AESGCM_MAC_SIZE, SGX_AESGCM_IV_SIZE,
                                                     NULL, 0,
                                                     (sgx_aes_gcm_128bit_tag_t *)encr_message);
    return status;
}