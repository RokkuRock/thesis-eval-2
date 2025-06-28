x509_verify_ctx_add_chain(struct x509_verify_ctx *ctx,
    struct x509_verify_chain *chain, char *name)
{
	size_t depth;
	X509 *last = x509_verify_chain_last(chain);
	X509 *leaf = x509_verify_chain_leaf(chain);
	depth = sk_X509_num(chain->certs);
	if (depth > 0)
		depth--;
	if (ctx->chains_count >= ctx->max_chains)
		return x509_verify_cert_error(ctx, last, depth,
		    X509_V_ERR_CERT_CHAIN_TOO_LONG, 0);
	if (chain->cert_errors[depth] ==
	    X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY)
		chain->cert_errors[depth] = X509_V_OK;
	if (!x509_verify_ctx_validate_legacy_chain(ctx, chain, depth))
		return 0;
	if ((ctx->chains[ctx->chains_count] = x509_verify_chain_dup(chain)) ==
	    NULL) {
		return x509_verify_cert_error(ctx, last, depth,
		    X509_V_ERR_OUT_OF_MEM, 0);
	}
	if (!x509_verify_cert_valid(ctx, leaf, NULL))
		return 0;
	if (!x509_verify_cert_hostname(ctx, leaf, name))
		return 0;
	ctx->chains_count++;
	ctx->error = X509_V_OK;
	ctx->error_depth = depth;
	return 1;
}