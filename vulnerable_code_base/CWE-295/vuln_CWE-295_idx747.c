NOEXPORT int verify_callback(int preverify_ok, X509_STORE_CTX *callback_ctx) {
    SSL *ssl;
    CLI *c;
    ssl=X509_STORE_CTX_get_ex_data(callback_ctx,
        SSL_get_ex_data_X509_STORE_CTX_idx());
    c=SSL_get_ex_data(ssl, index_ssl_cli);
    if(!c->opt->option.verify_chain && !c->opt->option.verify_peer) {
        s_log(LOG_INFO, "Certificate verification disabled");
        return 1;  
    }
    if(verify_checks(c, preverify_ok, callback_ctx)) {
        SSL_SESSION *sess=SSL_get1_session(c->ssl);
        if(sess) {
            int ok=SSL_SESSION_set_ex_data(sess, index_session_authenticated,
                (void *)(-1));
            SSL_SESSION_free(sess);
            if(!ok) {
                sslerror("SSL_SESSION_set_ex_data");
                return 0;  
            }
        }
        return 1;  
    }
    if(c->opt->option.client || c->opt->protocol)
        return 0;  
    if(c->opt->redirect_addr.names)
        return 1;  
    return 0;  
}