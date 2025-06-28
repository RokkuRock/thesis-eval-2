x509_verify(struct x509_verify_ctx *ctx, X509 *leaf, char *name)
{
	struct x509_verify_chain *current_chain;
	int retry_chain_build, full_chain = 0;
	if (ctx->roots == NULL || ctx->max_depth == 0) {
		ctx->error = X509_V_ERR_INVALID_CALL;
		goto err;
	}
	if (ctx->xsc != NULL) {
		if (leaf != NULL || name != NULL) {
			ctx->error = X509_V_ERR_INVALID_CALL;
			goto err;
		}
		leaf = ctx->xsc->cert;
		full_chain = 1;
		if (ctx->xsc->param->flags & X509_V_FLAG_PARTIAL_CHAIN)
			full_chain = 0;
		if ((ctx->xsc->chain = sk_X509_new_null()) == NULL) {
			ctx->error = X509_V_ERR_OUT_OF_MEM;
			goto err;
		}
		if (!X509_up_ref(leaf)) {
			ctx->error = X509_V_ERR_OUT_OF_MEM;
			goto err;
		}
		if (!sk_X509_push(ctx->xsc->chain, leaf)) {
			X509_free(leaf);
			ctx->error = X509_V_ERR_OUT_OF_MEM;
			goto err;
		}
		ctx->xsc->error_depth = 0;
		ctx->xsc->current_cert = leaf;
	}
	if (!x509_verify_cert_valid(ctx, leaf, NULL))
		goto err;
	if (!x509_verify_cert_hostname(ctx, leaf, name))
		goto err;
	if ((current_chain = x509_verify_chain_new()) == NULL) {
		ctx->error = X509_V_ERR_OUT_OF_MEM;
		goto err;
	}
	if (!x509_verify_chain_append(current_chain, leaf, &ctx->error)) {
		x509_verify_chain_free(current_chain);
		goto err;
	}
	do {
		retry_chain_build = 0;
		if (x509_verify_ctx_cert_is_root(ctx, leaf, full_chain)) {
			if (!x509_verify_ctx_add_chain(ctx, current_chain)) {
				x509_verify_chain_free(current_chain);
				goto err;
			}
		} else {
			x509_verify_build_chains(ctx, leaf, current_chain,
			    full_chain);
			if (full_chain && ctx->chains_count == 0) {
				if (!x509_verify_ctx_save_xsc_error(ctx)) {
					x509_verify_chain_free(current_chain);
					goto err;
				}
				full_chain = 0;
				retry_chain_build = 1;
			}
		}
	} while (retry_chain_build);
	x509_verify_chain_free(current_chain);
	if (!x509_verify_ctx_restore_xsc_error(ctx))
		goto err;
	if (ctx->chains_count == 0 && ctx->error == X509_V_OK) {
		ctx->error = X509_V_ERR_UNSPECIFIED;
		if (ctx->xsc != NULL && ctx->xsc->error != X509_V_OK)
			ctx->error = ctx->xsc->error;
	}
	if (ctx->chains_count > 0)
		ctx->error = X509_V_OK;
	if (ctx->xsc != NULL) {
		ctx->xsc->error = ctx->error;
		if (ctx->chains_count > 0) {
			if (!x509_verify_ctx_set_xsc_chain(ctx, ctx->chains[0],
			    1, 1))
				goto err;
			ctx->xsc->error = X509_V_OK;
			if(!x509_vfy_callback_indicate_success(ctx->xsc)) {
				ctx->error = ctx->xsc->error;
				goto err;
			}
		} else {
			if (ctx->xsc->verify_cb(0, ctx->xsc)) {
				ctx->xsc->error = X509_V_OK;
				return 1;
			}
		}
	}
	return (ctx->chains_count);
 err:
	if (ctx->error == X509_V_OK)
		ctx->error = X509_V_ERR_UNSPECIFIED;
	if (ctx->xsc != NULL)
		ctx->xsc->error = ctx->error;
	return 0;
}