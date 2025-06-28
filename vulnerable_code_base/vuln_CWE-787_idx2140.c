void trustedEnclaveInit(uint32_t _logLevel) {
    CALL_ONCE
    LOG_INFO(__FUNCTION__);
    globalLogLevel_ = _logLevel;
    oc_realloc_func = &reallocate_function;
    oc_free_func = &free_function;
    LOG_INFO("Setting memory functions");
    mp_get_memory_functions(NULL, &gmp_realloc_func, &gmp_free_func);
    mp_set_memory_functions(NULL, oc_realloc_func, oc_free_func);
    LOG_INFO("Calling enclave init");
    enclave_init();
    LOG_INFO("Reading random");
    globalRandom = calloc(32,1);
    int ret = sgx_read_rand(globalRandom, 32);
    if(ret != SGX_SUCCESS)
    {
        LOG_ERROR("sgx_read_rand failed. Aboring enclave.");
        abort();
    }
    LOG_INFO("Successfully inited enclave. Signed enclave version:" SIGNED_ENCLAVE_VERSION );
#ifndef SGX_DEBUG
    LOG_INFO("SECURITY WARNING: sgxwallet is running in INSECURE DEBUG MODE! NEVER USE IN PRODUCTION!");
#endif
#if SGX_DEBUG != 0
    LOG_INFO("SECURITY WARNING: sgxwallet is running in INSECURE DEBUG MODE! NEVER USE IN PRODUCTION!");
#endif
#if SGX_MODE == SIM
    LOG_INFO("SECURITY WARNING: sgxwallet is running in INSECURE SIMULATION MODE! NEVER USE IN PRODUCTION!");
#endif
}