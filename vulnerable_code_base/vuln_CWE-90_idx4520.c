static char *get_cert_prompt(TALLOC_CTX *mem_ctx,
                             struct cert_auth_info *cert_info)
{
    int ret;
    struct sss_certmap_ctx *ctx = NULL;
    unsigned char *der = NULL;
    size_t der_size;
    char *prompt = NULL;
    char *filter = NULL;
    char **domains = NULL;
    ret = sss_certmap_init(mem_ctx, NULL, NULL, &ctx);
    if (ret != 0) {
        DEBUG(SSSDBG_OP_FAILURE, "sss_certmap_init failed.\n");
        return NULL;
    }
    ret = sss_certmap_add_rule(ctx, 10, "KRB5:<ISSUER>.*",
                               "LDAP:{subject_dn!nss}", NULL);
    if (ret != 0) {
        DEBUG(SSSDBG_OP_FAILURE, "sss_certmap_add_rule failed.\n");
        goto done;
    }
    der = sss_base64_decode(mem_ctx, sss_cai_get_cert(cert_info), &der_size);
    if (der == NULL) {
        DEBUG(SSSDBG_OP_FAILURE, "sss_base64_decode failed.\n");
        goto done;
    }
    ret = sss_certmap_get_search_filter(ctx, der, der_size, &filter, &domains);
    if (ret != 0) {
        DEBUG(SSSDBG_OP_FAILURE, "sss_certmap_get_search_filter failed.\n");
        goto done;
    }
    prompt = talloc_asprintf(mem_ctx, "%s\n%s", sss_cai_get_label(cert_info),
                                                filter);
    if (prompt == NULL) {
        DEBUG(SSSDBG_OP_FAILURE, "talloc_strdup failed.\n");
    }
done:
    sss_certmap_free_filter_and_domains(filter, domains);
    sss_certmap_free_ctx(ctx);
    talloc_free(der);
    return prompt;
}