static void oidc_scrub_headers(request_rec *r) {
	oidc_cfg *cfg = ap_get_module_config(r->server->module_config,
			&auth_openidc_module);
	if (cfg->scrub_request_headers != 0) {
		oidc_scrub_request_headers(r, OIDC_DEFAULT_HEADER_PREFIX,
				oidc_cfg_dir_authn_header(r));
		if ((strstr(cfg->claim_prefix, OIDC_DEFAULT_HEADER_PREFIX)
				!= cfg->claim_prefix)) {
			oidc_scrub_request_headers(r, cfg->claim_prefix, NULL);
		}
	}
}