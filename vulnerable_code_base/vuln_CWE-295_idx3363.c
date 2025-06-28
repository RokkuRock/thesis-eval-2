x509_vfy_callback_indicate_success(X509_STORE_CTX *ctx)
{
	return x509_vfy_internal_verify(ctx, 1);
}