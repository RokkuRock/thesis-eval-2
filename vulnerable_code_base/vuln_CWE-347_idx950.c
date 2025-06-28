int x509_verify(const CA_CERT_CTX *ca_cert_ctx, const X509_CTX *cert, 
        int *pathLenConstraint) 
{
    int ret = X509_OK, i = 0;
    bigint *cert_sig;
    X509_CTX *next_cert = NULL;
    BI_CTX *ctx = NULL;
    bigint *mod = NULL, *expn = NULL;
    int match_ca_cert = 0;
    struct timeval tv;
    uint8_t is_self_signed = 0;
    if (cert == NULL)
    {
        ret = X509_VFY_ERROR_NO_TRUSTED_CERT;       
        goto end_verify;
    }
    if (asn1_compare_dn(cert->ca_cert_dn, cert->cert_dn) == 0)
    {
        is_self_signed = 1;
        ctx = cert->rsa_ctx->bi_ctx;
        mod = cert->rsa_ctx->m;
        expn = cert->rsa_ctx->e;
    }
    gettimeofday(&tv, NULL);
    if (tv.tv_sec < cert->not_before)
    {
        ret = X509_VFY_ERROR_NOT_YET_VALID;
        goto end_verify;
    }
    if (tv.tv_sec > cert->not_after)
    {
        ret = X509_VFY_ERROR_EXPIRED;
        goto end_verify;
    }
    if (cert->basic_constraint_present)
    {
        if (!cert->basic_constraint_cA &&
                IS_SET_KEY_USAGE_FLAG(cert, KEY_USAGE_KEY_CERT_SIGN))
        {
            ret = X509_VFY_ERROR_BASIC_CONSTRAINT;
            goto end_verify;
        }
        if (cert->basic_constraint_cA &&
            (!cert->key_usage_present || 
                IS_SET_KEY_USAGE_FLAG(cert, KEY_USAGE_KEY_CERT_SIGN)) &&
            (cert->basic_constraint_pathLenConstraint+1) < *pathLenConstraint)
        {
            ret = X509_VFY_ERROR_BASIC_CONSTRAINT;
            goto end_verify;
        }
    }
    next_cert = cert->next;
    if (next_cert == NULL)
    {
       if (ca_cert_ctx != NULL) 
       {
            while (i < CONFIG_X509_MAX_CA_CERTS && ca_cert_ctx->cert[i])
            {
                if (cert->basic_constraint_present && 
                        !ca_cert_ctx->cert[i]->basic_constraint_cA)
                    continue;
                if (asn1_compare_dn(cert->ca_cert_dn,
                                            ca_cert_ctx->cert[i]->cert_dn) == 0)
                {
                    match_ca_cert = true;
                    ctx = ca_cert_ctx->cert[i]->rsa_ctx->bi_ctx;
                    mod = ca_cert_ctx->cert[i]->rsa_ctx->m;
                    expn = ca_cert_ctx->cert[i]->rsa_ctx->e;
                    break;
                }
                i++;
            }
        }
        if (!match_ca_cert && !is_self_signed)
        {
            ret = X509_VFY_ERROR_NO_TRUSTED_CERT;       
            goto end_verify;
        }
    }
    else if (asn1_compare_dn(cert->ca_cert_dn, next_cert->cert_dn) != 0)
    {
        ret = X509_VFY_ERROR_INVALID_CHAIN;
        goto end_verify;
    }
    else  
    {
        ctx = next_cert->rsa_ctx->bi_ctx;
        mod = next_cert->rsa_ctx->m;
        expn = next_cert->rsa_ctx->e;
    }
    if (!match_ca_cert && is_self_signed)
    {
        ret = X509_VFY_ERROR_SELF_SIGNED;
        goto end_verify;
    }
    cert_sig = sig_verify(ctx, cert->signature, cert->sig_len, 
                        bi_clone(ctx, mod), bi_clone(ctx, expn));
    if (cert_sig && cert->digest)
    {
        if (bi_compare(cert_sig, cert->digest) != 0)
            ret = X509_VFY_ERROR_BAD_SIGNATURE;
        bi_free(ctx, cert_sig);
    }
    else
    {
        ret = X509_VFY_ERROR_BAD_SIGNATURE;
    }
    bi_clear_cache(ctx);
    if (ret)
        goto end_verify;
    if (next_cert != NULL)
    {
        (*pathLenConstraint)++;  
        ret = x509_verify(ca_cert_ctx, next_cert, pathLenConstraint);
    }
end_verify:
    return ret;
}