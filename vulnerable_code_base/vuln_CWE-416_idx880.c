static CURLcode ossl_connect_step1(struct Curl_easy *data,
                                   struct connectdata *conn, int sockindex)
{
  CURLcode result = CURLE_OK;
  char *ciphers;
  SSL_METHOD_QUAL SSL_METHOD *req_method = NULL;
  X509_LOOKUP *lookup = NULL;
  curl_socket_t sockfd = conn->sock[sockindex];
  struct ssl_connect_data *connssl = &conn->ssl[sockindex];
  ctx_option_t ctx_options = 0;
#ifdef SSL_CTRL_SET_TLSEXT_HOSTNAME
  bool sni;
  const char * const hostname = SSL_HOST_NAME();
#ifdef ENABLE_IPV6
  struct in6_addr addr;
#else
  struct in_addr addr;
#endif
#endif
  const long int ssl_version = SSL_CONN_CONFIG(version);
#ifdef USE_OPENSSL_SRP
  const enum CURL_TLSAUTH ssl_authtype = SSL_SET_OPTION(authtype);
#endif
  char * const ssl_cert = SSL_SET_OPTION(primary.clientcert);
  const struct curl_blob *ssl_cert_blob = SSL_SET_OPTION(primary.cert_blob);
  const struct curl_blob *ca_info_blob = SSL_CONN_CONFIG(ca_info_blob);
  const char * const ssl_cert_type = SSL_SET_OPTION(cert_type);
  const char * const ssl_cafile =
    (ca_info_blob ? NULL : SSL_CONN_CONFIG(CAfile));
  const char * const ssl_capath = SSL_CONN_CONFIG(CApath);
  const bool verifypeer = SSL_CONN_CONFIG(verifypeer);
  const char * const ssl_crlfile = SSL_SET_OPTION(CRLfile);
  char error_buffer[256];
  struct ssl_backend_data *backend = connssl->backend;
  bool imported_native_ca = false;
  DEBUGASSERT(ssl_connect_1 == connssl->connecting_state);
  result = ossl_seed(data);
  if(result)
    return result;
  SSL_SET_OPTION_LVALUE(certverifyresult) = !X509_V_OK;
  switch(ssl_version) {
  case CURL_SSLVERSION_DEFAULT:
  case CURL_SSLVERSION_TLSv1:
  case CURL_SSLVERSION_TLSv1_0:
  case CURL_SSLVERSION_TLSv1_1:
  case CURL_SSLVERSION_TLSv1_2:
  case CURL_SSLVERSION_TLSv1_3:
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L)
    req_method = TLS_client_method();
#else
    req_method = SSLv23_client_method();
#endif
    use_sni(TRUE);
    break;
  case CURL_SSLVERSION_SSLv2:
    failf(data, "No SSLv2 support");
    return CURLE_NOT_BUILT_IN;
  case CURL_SSLVERSION_SSLv3:
    failf(data, "No SSLv3 support");
    return CURLE_NOT_BUILT_IN;
  default:
    failf(data, "Unrecognized parameter passed via CURLOPT_SSLVERSION");
    return CURLE_SSL_CONNECT_ERROR;
  }
  if(backend->ctx)
    SSL_CTX_free(backend->ctx);
  backend->ctx = SSL_CTX_new(req_method);
  if(!backend->ctx) {
    failf(data, "SSL: couldn't create a context: %s",
          ossl_strerror(ERR_peek_error(), error_buffer, sizeof(error_buffer)));
    return CURLE_OUT_OF_MEMORY;
  }
#ifdef SSL_MODE_RELEASE_BUFFERS
  SSL_CTX_set_mode(backend->ctx, SSL_MODE_RELEASE_BUFFERS);
#endif
#ifdef SSL_CTRL_SET_MSG_CALLBACK
  if(data->set.fdebug && data->set.verbose) {
    SSL_CTX_set_msg_callback(backend->ctx, ossl_trace);
    SSL_CTX_set_msg_callback_arg(backend->ctx, conn);
    set_logger(conn, data);
  }
#endif
  ctx_options = SSL_OP_ALL;
#ifdef SSL_OP_NO_TICKET
  ctx_options |= SSL_OP_NO_TICKET;
#endif
#ifdef SSL_OP_NO_COMPRESSION
  ctx_options |= SSL_OP_NO_COMPRESSION;
#endif
#ifdef SSL_OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG
  ctx_options &= ~SSL_OP_NETSCAPE_REUSE_CIPHER_CHANGE_BUG;
#endif
#ifdef SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS
  if(!SSL_SET_OPTION(enable_beast))
    ctx_options &= ~SSL_OP_DONT_INSERT_EMPTY_FRAGMENTS;
#endif
  switch(ssl_version) {
    case CURL_SSLVERSION_SSLv2:
    case CURL_SSLVERSION_SSLv3:
      return CURLE_NOT_BUILT_IN;
    case CURL_SSLVERSION_DEFAULT:
    case CURL_SSLVERSION_TLSv1:  
    case CURL_SSLVERSION_TLSv1_0:  
    case CURL_SSLVERSION_TLSv1_1:  
    case CURL_SSLVERSION_TLSv1_2:  
    case CURL_SSLVERSION_TLSv1_3:  
      ctx_options |= SSL_OP_NO_SSLv2;
      ctx_options |= SSL_OP_NO_SSLv3;
#if (OPENSSL_VERSION_NUMBER >= 0x10100000L)  
      result = set_ssl_version_min_max(backend->ctx, conn);
#else
      result = set_ssl_version_min_max_legacy(&ctx_options, data, conn,
                                              sockindex);
#endif
      if(result != CURLE_OK)
        return result;
      break;
    default:
      failf(data, "Unrecognized parameter passed via CURLOPT_SSLVERSION");
      return CURLE_SSL_CONNECT_ERROR;
  }
  SSL_CTX_set_options(backend->ctx, ctx_options);
#ifdef HAS_NPN
  if(conn->bits.tls_enable_npn)
    SSL_CTX_set_next_proto_select_cb(backend->ctx, select_next_proto_cb, data);
#endif
#ifdef HAS_ALPN
  if(conn->bits.tls_enable_alpn) {
    int cur = 0;
    unsigned char protocols[128];
#ifdef USE_HTTP2
    if(data->state.httpwant >= CURL_HTTP_VERSION_2
#ifndef CURL_DISABLE_PROXY
       && (!SSL_IS_PROXY() || !conn->bits.tunnel_proxy)
#endif
      ) {
      protocols[cur++] = ALPN_H2_LENGTH;
      memcpy(&protocols[cur], ALPN_H2, ALPN_H2_LENGTH);
      cur += ALPN_H2_LENGTH;
      infof(data, "ALPN, offering %s\n", ALPN_H2);
    }
#endif
    protocols[cur++] = ALPN_HTTP_1_1_LENGTH;
    memcpy(&protocols[cur], ALPN_HTTP_1_1, ALPN_HTTP_1_1_LENGTH);
    cur += ALPN_HTTP_1_1_LENGTH;
    infof(data, "ALPN, offering %s\n", ALPN_HTTP_1_1);
    if(SSL_CTX_set_alpn_protos(backend->ctx, protocols, cur)) {
      failf(data, "Error setting ALPN");
      return CURLE_SSL_CONNECT_ERROR;
    }
  }
#endif
  if(ssl_cert || ssl_cert_blob || ssl_cert_type) {
    if(!result &&
       !cert_stuff(data, backend->ctx,
                   ssl_cert, ssl_cert_blob, ssl_cert_type,
                   SSL_SET_OPTION(key), SSL_SET_OPTION(key_blob),
                   SSL_SET_OPTION(key_type), SSL_SET_OPTION(key_passwd)))
      result = CURLE_SSL_CERTPROBLEM;
    if(result)
      return result;
  }
  ciphers = SSL_CONN_CONFIG(cipher_list);
  if(!ciphers)
    ciphers = (char *)DEFAULT_CIPHER_SELECTION;
  if(ciphers) {
    if(!SSL_CTX_set_cipher_list(backend->ctx, ciphers)) {
      failf(data, "failed setting cipher list: %s", ciphers);
      return CURLE_SSL_CIPHER;
    }
    infof(data, "Cipher selection: %s\n", ciphers);
  }
#ifdef HAVE_SSL_CTX_SET_CIPHERSUITES
  {
    char *ciphers13 = SSL_CONN_CONFIG(cipher_list13);
    if(ciphers13) {
      if(!SSL_CTX_set_ciphersuites(backend->ctx, ciphers13)) {
        failf(data, "failed setting TLS 1.3 cipher suite: %s", ciphers13);
        return CURLE_SSL_CIPHER;
      }
      infof(data, "TLS 1.3 cipher selection: %s\n", ciphers13);
    }
  }
#endif
#ifdef HAVE_SSL_CTX_SET_POST_HANDSHAKE_AUTH
  SSL_CTX_set_post_handshake_auth(backend->ctx, 1);
#endif
#ifdef HAVE_SSL_CTX_SET_EC_CURVES
  {
    char *curves = SSL_CONN_CONFIG(curves);
    if(curves) {
      if(!SSL_CTX_set1_curves_list(backend->ctx, curves)) {
        failf(data, "failed setting curves list: '%s'", curves);
        return CURLE_SSL_CIPHER;
      }
    }
  }
#endif
#ifdef USE_OPENSSL_SRP
  if(ssl_authtype == CURL_TLSAUTH_SRP) {
    char * const ssl_username = SSL_SET_OPTION(username);
    infof(data, "Using TLS-SRP username: %s\n", ssl_username);
    if(!SSL_CTX_set_srp_username(backend->ctx, ssl_username)) {
      failf(data, "Unable to set SRP user name");
      return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    if(!SSL_CTX_set_srp_password(backend->ctx, SSL_SET_OPTION(password))) {
      failf(data, "failed setting SRP password");
      return CURLE_BAD_FUNCTION_ARGUMENT;
    }
    if(!SSL_CONN_CONFIG(cipher_list)) {
      infof(data, "Setting cipher list SRP\n");
      if(!SSL_CTX_set_cipher_list(backend->ctx, "SRP")) {
        failf(data, "failed setting SRP cipher list");
        return CURLE_SSL_CIPHER;
      }
    }
  }
#endif
#if defined(USE_WIN32_CRYPTO)
  if((SSL_CONN_CONFIG(verifypeer) || SSL_CONN_CONFIG(verifyhost)) &&
     (SSL_SET_OPTION(native_ca_store))) {
    X509_STORE *store = SSL_CTX_get_cert_store(backend->ctx);
    HCERTSTORE hStore = CertOpenSystemStore(0, TEXT("ROOT"));
    if(hStore) {
      PCCERT_CONTEXT pContext = NULL;
      CERT_ENHKEY_USAGE *enhkey_usage = NULL;
      DWORD enhkey_usage_size = 0;
      result = CURLE_OK;
      for(;;) {
        X509 *x509;
        FILETIME now;
        BYTE key_usage[2];
        DWORD req_size;
        const unsigned char *encoded_cert;
#if defined(DEBUGBUILD) && !defined(CURL_DISABLE_VERBOSE_STRINGS)
        char cert_name[256];
#endif
        pContext = CertEnumCertificatesInStore(hStore, pContext);
        if(!pContext)
          break;
#if defined(DEBUGBUILD) && !defined(CURL_DISABLE_VERBOSE_STRINGS)
        if(!CertGetNameStringA(pContext, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0,
                               NULL, cert_name, sizeof(cert_name))) {
          strcpy(cert_name, "Unknown");
        }
        infof(data, "SSL: Checking cert \"%s\"\n", cert_name);
#endif
        encoded_cert = (const unsigned char *)pContext->pbCertEncoded;
        if(!encoded_cert)
          continue;
        GetSystemTimeAsFileTime(&now);
        if(CompareFileTime(&pContext->pCertInfo->NotBefore, &now) > 0 ||
           CompareFileTime(&now, &pContext->pCertInfo->NotAfter) > 0)
          continue;
        if(CertGetIntendedKeyUsage(pContext->dwCertEncodingType,
                                   pContext->pCertInfo,
                                   key_usage, sizeof(key_usage))) {
          if(!(key_usage[0] & CERT_KEY_CERT_SIGN_KEY_USAGE))
            continue;
        }
        else if(GetLastError())
          continue;
        if(CertGetEnhancedKeyUsage(pContext, 0, NULL, &req_size)) {
          if(req_size && req_size > enhkey_usage_size) {
            void *tmp = realloc(enhkey_usage, req_size);
            if(!tmp) {
              failf(data, "SSL: Out of memory allocating for OID list");
              result = CURLE_OUT_OF_MEMORY;
              break;
            }
            enhkey_usage = (CERT_ENHKEY_USAGE *)tmp;
            enhkey_usage_size = req_size;
          }
          if(CertGetEnhancedKeyUsage(pContext, 0, enhkey_usage, &req_size)) {
            if(!enhkey_usage->cUsageIdentifier) {
              if((HRESULT)GetLastError() != CRYPT_E_NOT_FOUND)
                continue;
            }
            else {
              DWORD i;
              bool found = false;
              for(i = 0; i < enhkey_usage->cUsageIdentifier; ++i) {
                if(!strcmp("1.3.6.1.5.5.7.3.1"  ,
                           enhkey_usage->rgpszUsageIdentifier[i])) {
                  found = true;
                  break;
                }
              }
              if(!found)
                continue;
            }
          }
          else
            continue;
        }
        else
          continue;
        x509 = d2i_X509(NULL, &encoded_cert, pContext->cbCertEncoded);
        if(!x509)
          continue;
        if(X509_STORE_add_cert(store, x509) == 1) {
#if defined(DEBUGBUILD) && !defined(CURL_DISABLE_VERBOSE_STRINGS)
          infof(data, "SSL: Imported cert \"%s\"\n", cert_name);
#endif
          imported_native_ca = true;
        }
        X509_free(x509);
      }
      free(enhkey_usage);
      CertFreeCertificateContext(pContext);
      CertCloseStore(hStore, 0);
      if(result)
        return result;
    }
    if(imported_native_ca)
      infof(data, "successfully imported windows ca store\n");
    else
      infof(data, "error importing windows ca store, continuing anyway\n");
  }
#endif
  if(ca_info_blob) {
    result = load_cacert_from_memory(backend->ctx, ca_info_blob);
    if(result) {
      if(result == CURLE_OUT_OF_MEMORY ||
         (verifypeer && !imported_native_ca)) {
        failf(data, "error importing CA certificate blob");
        return result;
      }
      infof(data, "error importing CA certificate blob, continuing anyway\n");
    }
  }
#if defined(OPENSSL_VERSION_MAJOR) && (OPENSSL_VERSION_MAJOR >= 3)
  {
    if(ssl_cafile) {
      if(!SSL_CTX_load_verify_file(backend->ctx, ssl_cafile)) {
        if(verifypeer && !imported_native_ca) {
          failf(data, "error setting certificate file: %s", ssl_cafile);
          return CURLE_SSL_CACERT_BADFILE;
        }
        infof(data, "error setting certificate file, continuing anyway\n");
      }
      infof(data, " CAfile: %s\n", ssl_cafile);
    }
    if(ssl_capath) {
      if(!SSL_CTX_load_verify_dir(backend->ctx, ssl_capath)) {
        if(verifypeer && !imported_native_ca) {
          failf(data, "error setting certificate path: %s", ssl_capath);
          return CURLE_SSL_CACERT_BADFILE;
        }
        infof(data, "error setting certificate path, continuing anyway\n");
      }
      infof(data, " CApath: %s\n", ssl_capath);
    }
  }
#else
  if(ssl_cafile || ssl_capath) {
    if(!SSL_CTX_load_verify_locations(backend->ctx, ssl_cafile, ssl_capath)) {
      if(verifypeer && !imported_native_ca) {
        failf(data, "error setting certificate verify locations:"
              "  CAfile: %s CApath: %s",
              ssl_cafile ? ssl_cafile : "none",
              ssl_capath ? ssl_capath : "none");
        return CURLE_SSL_CACERT_BADFILE;
      }
      infof(data, "error setting certificate verify locations,"
            " continuing anyway:\n");
    }
    else {
      infof(data, "successfully set certificate verify locations:\n");
    }
    infof(data, " CAfile: %s\n", ssl_cafile ? ssl_cafile : "none");
    infof(data, " CApath: %s\n", ssl_capath ? ssl_capath : "none");
  }
#endif
#ifdef CURL_CA_FALLBACK
  if(verifypeer &&
     !ca_info_blob && !ssl_cafile && !ssl_capath && !imported_native_ca) {
    SSL_CTX_set_default_verify_paths(backend->ctx);
  }
#endif
  if(ssl_crlfile) {
    lookup = X509_STORE_add_lookup(SSL_CTX_get_cert_store(backend->ctx),
                                 X509_LOOKUP_file());
    if(!lookup ||
       (!X509_load_crl_file(lookup, ssl_crlfile, X509_FILETYPE_PEM)) ) {
      failf(data, "error loading CRL file: %s", ssl_crlfile);
      return CURLE_SSL_CRL_BADFILE;
    }
    infof(data, "successfully load CRL file:\n");
    X509_STORE_set_flags(SSL_CTX_get_cert_store(backend->ctx),
                         X509_V_FLAG_CRL_CHECK|X509_V_FLAG_CRL_CHECK_ALL);
    infof(data, "  CRLfile: %s\n", ssl_crlfile);
  }
  if(verifypeer) {
#if defined(X509_V_FLAG_TRUSTED_FIRST)
    X509_STORE_set_flags(SSL_CTX_get_cert_store(backend->ctx),
                         X509_V_FLAG_TRUSTED_FIRST);
#endif
#ifdef X509_V_FLAG_PARTIAL_CHAIN
    if(!SSL_SET_OPTION(no_partialchain) && !ssl_crlfile) {
      X509_STORE_set_flags(SSL_CTX_get_cert_store(backend->ctx),
                           X509_V_FLAG_PARTIAL_CHAIN);
    }
#endif
  }
  SSL_CTX_set_verify(backend->ctx,
                     verifypeer ? SSL_VERIFY_PEER : SSL_VERIFY_NONE, NULL);
#ifdef HAVE_KEYLOG_CALLBACK
  if(Curl_tls_keylog_enabled()) {
    SSL_CTX_set_keylog_callback(backend->ctx, ossl_keylog_callback);
  }
#endif
  SSL_CTX_set_session_cache_mode(backend->ctx,
      SSL_SESS_CACHE_CLIENT | SSL_SESS_CACHE_NO_INTERNAL);
  SSL_CTX_sess_set_new_cb(backend->ctx, ossl_new_session_cb);
  if(data->set.ssl.fsslctx) {
    Curl_set_in_callback(data, true);
    result = (*data->set.ssl.fsslctx)(data, backend->ctx,
                                      data->set.ssl.fsslctxp);
    Curl_set_in_callback(data, false);
    if(result) {
      failf(data, "error signaled by ssl ctx callback");
      return result;
    }
  }
  if(backend->handle)
    SSL_free(backend->handle);
  backend->handle = SSL_new(backend->ctx);
  if(!backend->handle) {
    failf(data, "SSL: couldn't create a context (handle)!");
    return CURLE_OUT_OF_MEMORY;
  }
#if (OPENSSL_VERSION_NUMBER >= 0x0090808fL) && !defined(OPENSSL_NO_TLSEXT) && \
    !defined(OPENSSL_NO_OCSP)
  if(SSL_CONN_CONFIG(verifystatus))
    SSL_set_tlsext_status_type(backend->handle, TLSEXT_STATUSTYPE_ocsp);
#endif
#if defined(OPENSSL_IS_BORINGSSL) && defined(ALLOW_RENEG)
  SSL_set_renegotiate_mode(backend->handle, ssl_renegotiate_freely);
#endif
  SSL_set_connect_state(backend->handle);
  backend->server_cert = 0x0;
#ifdef SSL_CTRL_SET_TLSEXT_HOSTNAME
  if((0 == Curl_inet_pton(AF_INET, hostname, &addr)) &&
#ifdef ENABLE_IPV6
     (0 == Curl_inet_pton(AF_INET6, hostname, &addr)) &&
#endif
     sni) {
    size_t nlen = strlen(hostname);
    if((long)nlen >= data->set.buffer_size)
      return CURLE_SSL_CONNECT_ERROR;
    Curl_strntolower(data->state.buffer, hostname, nlen);
    data->state.buffer[nlen] = 0;
    if(!SSL_set_tlsext_host_name(backend->handle, data->state.buffer))
      infof(data, "WARNING: failed to configure server name indication (SNI) "
            "TLS extension\n");
  }
#endif
  if(SSL_SET_OPTION(primary.sessionid)) {
    void *ssl_sessionid = NULL;
    int data_idx = ossl_get_ssl_data_index();
    int connectdata_idx = ossl_get_ssl_conn_index();
    int sockindex_idx = ossl_get_ssl_sockindex_index();
    int proxy_idx = ossl_get_proxy_index();
    if(data_idx >= 0 && connectdata_idx >= 0 && sockindex_idx >= 0 &&
       proxy_idx >= 0) {
      SSL_set_ex_data(backend->handle, data_idx, data);
      SSL_set_ex_data(backend->handle, connectdata_idx, conn);
      SSL_set_ex_data(backend->handle, sockindex_idx, conn->sock + sockindex);
#ifndef CURL_DISABLE_PROXY
      SSL_set_ex_data(backend->handle, proxy_idx, SSL_IS_PROXY() ? (void *) 1:
                      NULL);
#else
      SSL_set_ex_data(backend->handle, proxy_idx, NULL);
#endif
    }
    Curl_ssl_sessionid_lock(data);
    if(!Curl_ssl_getsessionid(data, conn, SSL_IS_PROXY() ? TRUE : FALSE,
                              &ssl_sessionid, NULL, sockindex)) {
      if(!SSL_set_session(backend->handle, ssl_sessionid)) {
        Curl_ssl_sessionid_unlock(data);
        failf(data, "SSL: SSL_set_session failed: %s",
              ossl_strerror(ERR_get_error(), error_buffer,
                            sizeof(error_buffer)));
        return CURLE_SSL_CONNECT_ERROR;
      }
      infof(data, "SSL re-using session ID\n");
    }
    Curl_ssl_sessionid_unlock(data);
  }
#ifndef CURL_DISABLE_PROXY
  if(conn->proxy_ssl[sockindex].use) {
    BIO *const bio = BIO_new(BIO_f_ssl());
    SSL *handle = conn->proxy_ssl[sockindex].backend->handle;
    DEBUGASSERT(ssl_connection_complete == conn->proxy_ssl[sockindex].state);
    DEBUGASSERT(handle != NULL);
    DEBUGASSERT(bio != NULL);
    BIO_set_ssl(bio, handle, FALSE);
    SSL_set_bio(backend->handle, bio, bio);
  }
  else
#endif
    if(!SSL_set_fd(backend->handle, (int)sockfd)) {
    failf(data, "SSL: SSL_set_fd failed: %s",
          ossl_strerror(ERR_get_error(), error_buffer, sizeof(error_buffer)));
    return CURLE_SSL_CONNECT_ERROR;
  }
  connssl->connecting_state = ssl_connect_2;
  return CURLE_OK;
}