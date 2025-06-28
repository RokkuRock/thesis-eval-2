cherokee_validator_ldap_check (cherokee_validator_ldap_t *ldap,
                               cherokee_connection_t     *conn)
{
	int                              re;
	ret_t                            ret;
	size_t                           size;
	char                            *dn;
	LDAPMessage                     *message;
	LDAPMessage                     *first;
	char                            *attrs[] = { LDAP_NO_ATTRS, NULL };
	cherokee_validator_ldap_props_t *props   = VAL_LDAP_PROP(ldap);
	if ((conn->validator == NULL) ||
	    cherokee_buffer_is_empty (&conn->validator->user))
		return ret_error;
	size = cherokee_buffer_cnt_cspn (&conn->validator->user, 0, "*()");
	if (size != conn->validator->user.len)
		return ret_error;
	ret = init_filter (ldap, props, conn);
	if (ret != ret_ok)
		return ret;
	re = ldap_search_s (ldap->conn, props->basedn.buf, LDAP_SCOPE_SUBTREE, ldap->filter.buf, attrs, 0, &message);
	if (re != LDAP_SUCCESS) {
		LOG_ERROR (CHEROKEE_ERROR_VALIDATOR_LDAP_SEARCH,
		           props->filter.buf ? props->filter.buf : "");
		return ret_error;
	}
	TRACE (ENTRIES, "subtree search (%s): done\n", ldap->filter.buf ? ldap->filter.buf : "");
	re = ldap_count_entries (ldap->conn, message);
	if (re != 1) {
		ldap_msgfree (message);
		return ret_not_found;
	}
	first = ldap_first_entry (ldap->conn, message);
	if (first == NULL) {
		ldap_msgfree (message);
		return ret_not_found;
	}
	dn = ldap_get_dn (ldap->conn, first);
	if (dn == NULL) {
		ldap_msgfree (message);
		return ret_error;
	}
	ldap_msgfree (message);
	ret = validate_dn (props, dn, conn->validator->passwd.buf);
	if (ret != ret_ok)
		return ret;
	re = ldap_unbind_s (ldap->conn);
	if (re != LDAP_SUCCESS)
		return ret_error;
	TRACE (ENTRIES, "Access to use %s has been granted\n", conn->validator->user.buf);
	return ret_ok;
}