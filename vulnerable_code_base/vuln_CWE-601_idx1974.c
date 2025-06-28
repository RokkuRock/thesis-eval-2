static int oidc_handle_discovery_response(request_rec *r, oidc_cfg *c) {
	char *issuer = NULL, *target_link_uri = NULL, *login_hint = NULL,
			*auth_request_params = NULL, *csrf_cookie, *csrf_query = NULL,
			*user = NULL, *path_scopes;
	oidc_provider_t *provider = NULL;
	oidc_util_get_request_parameter(r, OIDC_DISC_OP_PARAM, &issuer);
	oidc_util_get_request_parameter(r, OIDC_DISC_USER_PARAM, &user);
	oidc_util_get_request_parameter(r, OIDC_DISC_RT_PARAM, &target_link_uri);
	oidc_util_get_request_parameter(r, OIDC_DISC_LH_PARAM, &login_hint);
	oidc_util_get_request_parameter(r, OIDC_DISC_SC_PARAM, &path_scopes);
	oidc_util_get_request_parameter(r, OIDC_DISC_AR_PARAM,
			&auth_request_params);
	oidc_util_get_request_parameter(r, OIDC_CSRF_NAME, &csrf_query);
	csrf_cookie = oidc_util_get_cookie(r, OIDC_CSRF_NAME);
	if (csrf_cookie) {
		oidc_util_set_cookie(r, OIDC_CSRF_NAME, "", 0,
				OIDC_COOKIE_EXT_SAME_SITE_NONE(r));
		if ((csrf_query == NULL)
				|| apr_strnatcmp(csrf_query, csrf_cookie) != 0) {
			oidc_warn(r,
					"CSRF protection failed, no Discovery and dynamic client registration will be allowed");
			csrf_cookie = NULL;
		}
	}
	oidc_debug(r,
			"issuer=\"%s\", target_link_uri=\"%s\", login_hint=\"%s\", user=\"%s\"",
			issuer, target_link_uri, login_hint, user);
	if (target_link_uri == NULL) {
		if (c->default_sso_url == NULL) {
			return oidc_util_html_send_error(r, c->error_template,
					"Invalid Request",
					"SSO to this module without specifying a \"target_link_uri\" parameter is not possible because " OIDCDefaultURL " is not set.",
					HTTP_INTERNAL_SERVER_ERROR);
		}
		target_link_uri = c->default_sso_url;
	}
	if (oidc_target_link_uri_matches_configuration(r, c, target_link_uri)
			== FALSE) {
		return oidc_util_html_send_error(r, c->error_template,
				"Invalid Request",
				"\"target_link_uri\" parameter does not match configuration settings, aborting to prevent an open redirect.",
				HTTP_UNAUTHORIZED);
	}
	if (c->metadata_dir == NULL) {
		if ((oidc_provider_static_config(r, c, &provider) == TRUE)
				&& (issuer != NULL)) {
			if (apr_strnatcmp(provider->issuer, issuer) != 0) {
				return oidc_util_html_send_error(r, c->error_template,
						"Invalid Request",
						apr_psprintf(r->pool,
								"The \"iss\" value must match the configured providers' one (%s != %s).",
								issuer, c->provider.issuer),
								HTTP_INTERNAL_SERVER_ERROR);
			}
		}
		return oidc_authenticate_user(r, c, NULL, target_link_uri, login_hint,
				NULL, NULL, auth_request_params, path_scopes);
	}
	if (user != NULL) {
		if (login_hint == NULL)
			login_hint = apr_pstrdup(r->pool, user);
		if (strstr(user, "https://") != user)
			user = apr_psprintf(r->pool, "https://%s", user);
		if (oidc_proto_url_based_discovery(r, c, user, &issuer) == FALSE) {
			return oidc_util_html_send_error(r, c->error_template,
					"Invalid Request",
					"Could not resolve the provided user identifier to an OpenID Connect provider; check your syntax.",
					HTTP_NOT_FOUND);
		}
	} else if (strstr(issuer, OIDC_STR_AT) != NULL) {
		if (login_hint == NULL) {
			login_hint = apr_pstrdup(r->pool, issuer);
		}
		if (oidc_proto_account_based_discovery(r, c, issuer, &issuer)
				== FALSE) {
			return oidc_util_html_send_error(r, c->error_template,
					"Invalid Request",
					"Could not resolve the provided account name to an OpenID Connect provider; check your syntax.",
					HTTP_NOT_FOUND);
		}
	}
	int n = strlen(issuer);
	if (issuer[n - 1] == OIDC_CHAR_FORWARD_SLASH)
		issuer[n - 1] = '\0';
	if ((oidc_metadata_get(r, c, issuer, &provider, csrf_cookie != NULL) == TRUE)
			&& (provider != NULL)) {
		return oidc_authenticate_user(r, c, provider, target_link_uri,
				login_hint, NULL, NULL, auth_request_params, path_scopes);
	}
	return oidc_util_html_send_error(r, c->error_template, "Invalid Request",
			"Could not find valid provider metadata for the selected OpenID Connect provider; contact the administrator",
			HTTP_NOT_FOUND);
}