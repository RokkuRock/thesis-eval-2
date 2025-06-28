validate_as_request(kdc_realm_t *kdc_active_realm,
                    register krb5_kdc_req *request, krb5_db_entry client,
                    krb5_db_entry server, krb5_timestamp kdc_time,
                    const char **status, krb5_pa_data ***e_data)
{
    int errcode;
    krb5_error_code ret;
    if (request->kdc_options & AS_INVALID_OPTIONS) {
        *status = "INVALID AS OPTIONS";
        return KDC_ERR_BADOPTION;
    }
    if (client.expiration && client.expiration < kdc_time) {
        *status = "CLIENT EXPIRED";
        if (vague_errors)
            return(KRB_ERR_GENERIC);
        else
            return(KDC_ERR_NAME_EXP);
    }
    if (client.pw_expiration && client.pw_expiration < kdc_time &&
        !isflagset(server.attributes, KRB5_KDB_PWCHANGE_SERVICE)) {
        *status = "CLIENT KEY EXPIRED";
        if (vague_errors)
            return(KRB_ERR_GENERIC);
        else
            return(KDC_ERR_KEY_EXP);
    }
    if (server.expiration && server.expiration < kdc_time) {
        *status = "SERVICE EXPIRED";
        return(KDC_ERR_SERVICE_EXP);
    }
    if (isflagset(client.attributes, KRB5_KDB_REQUIRES_PWCHANGE) &&
        !isflagset(server.attributes, KRB5_KDB_PWCHANGE_SERVICE)) {
        *status = "REQUIRED PWCHANGE";
        return(KDC_ERR_KEY_EXP);
    }
    if ((isflagset(request->kdc_options, KDC_OPT_ALLOW_POSTDATE) ||
         isflagset(request->kdc_options, KDC_OPT_POSTDATED)) &&
        (isflagset(client.attributes, KRB5_KDB_DISALLOW_POSTDATED) ||
         isflagset(server.attributes, KRB5_KDB_DISALLOW_POSTDATED))) {
        *status = "POSTDATE NOT ALLOWED";
        return(KDC_ERR_CANNOT_POSTDATE);
    }
    if (isflagset(request->kdc_options, KDC_OPT_PROXIABLE) &&
        (isflagset(client.attributes, KRB5_KDB_DISALLOW_PROXIABLE) ||
         isflagset(server.attributes, KRB5_KDB_DISALLOW_PROXIABLE))) {
        *status = "PROXIABLE NOT ALLOWED";
        return(KDC_ERR_POLICY);
    }
    if (isflagset(client.attributes, KRB5_KDB_DISALLOW_ALL_TIX)) {
        *status = "CLIENT LOCKED OUT";
        return(KDC_ERR_CLIENT_REVOKED);
    }
    if (isflagset(server.attributes, KRB5_KDB_DISALLOW_ALL_TIX)) {
        *status = "SERVICE LOCKED OUT";
        return(KDC_ERR_S_PRINCIPAL_UNKNOWN);
    }
    if (isflagset(server.attributes, KRB5_KDB_DISALLOW_SVR)) {
        *status = "SERVICE NOT ALLOWED";
        return(KDC_ERR_MUST_USE_USER2USER);
    }
    if (check_anon(kdc_active_realm, request->client, request->server) != 0) {
        *status = "ANONYMOUS NOT ALLOWED";
        return(KDC_ERR_POLICY);
    }
    ret = krb5_db_check_policy_as(kdc_context, request, &client, &server,
                                  kdc_time, status, e_data);
    if (ret && ret != KRB5_PLUGIN_OP_NOTSUPP)
        return errcode_to_protocol(ret);
    errcode = against_local_policy_as(request, client, server,
                                      kdc_time, status, e_data);
    if (errcode)
        return errcode;
    return 0;
}