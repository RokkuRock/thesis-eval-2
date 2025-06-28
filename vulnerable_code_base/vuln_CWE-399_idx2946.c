create_tls_session(int csock, int type   )
{
    int rc = 0;
    gnutls_session *session = gnutls_malloc(sizeof(gnutls_session));
    gnutls_init(session, type);
#  ifdef HAVE_GNUTLS_PRIORITY_SET_DIRECT
    gnutls_priority_set_direct(*session, "NORMAL:+ANON-DH", NULL);
#  else
    gnutls_set_default_priority(*session);
    gnutls_kx_set_priority(*session, tls_kx_order);
#  endif
    gnutls_transport_set_ptr(*session, (gnutls_transport_ptr) GINT_TO_POINTER(csock));
    switch (type) {
        case GNUTLS_SERVER:
            gnutls_credentials_set(*session, GNUTLS_CRD_ANON, anon_cred_s);
            break;
        case GNUTLS_CLIENT:
            gnutls_credentials_set(*session, GNUTLS_CRD_ANON, anon_cred_c);
            break;
    }
    do {
        rc = gnutls_handshake(*session);
    } while (rc == GNUTLS_E_INTERRUPTED || rc == GNUTLS_E_AGAIN);
    if (rc < 0) {
        crm_err("Handshake failed: %s", gnutls_strerror(rc));
        gnutls_deinit(*session);
        gnutls_free(session);
        return NULL;
    }
    return session;
}