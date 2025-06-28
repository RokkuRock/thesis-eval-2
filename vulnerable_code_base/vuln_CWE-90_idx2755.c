errno_t sss_filter_sanitize(TALLOC_CTX *mem_ctx,
                            const char *input,
                            char **sanitized)
{
    return sss_filter_sanitize_ex(mem_ctx, input, sanitized, NULL);
}