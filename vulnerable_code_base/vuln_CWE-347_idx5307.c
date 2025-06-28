static bigint *sig_verify(BI_CTX *ctx, const uint8_t *sig, int sig_len,
        bigint *modulus, bigint *pub_exp)
{
    int i, size;
    bigint *decrypted_bi, *dat_bi;
    bigint *bir = NULL;
    uint8_t *block = (uint8_t *)malloc(sig_len);
    dat_bi = bi_import(ctx, sig, sig_len);
    ctx->mod_offset = BIGINT_M_OFFSET;
    decrypted_bi = bi_mod_power2(ctx, dat_bi, modulus, pub_exp);
    bi_export(ctx, decrypted_bi, block, sig_len);
    ctx->mod_offset = BIGINT_M_OFFSET;
    i = 10;  
    while (block[i++] && i < sig_len);
    size = sig_len - i;
    if (size > 0)
    {
        int len;
        const uint8_t *sig_ptr = get_signature(&block[i], &len);
        if (sig_ptr)
        {
            bir = bi_import(ctx, sig_ptr, len);
        }
    }
    free(block);
    bi_clear_cache(ctx);
    return bir;
}