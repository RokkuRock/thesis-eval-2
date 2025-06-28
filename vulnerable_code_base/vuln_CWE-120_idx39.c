static int jsi_wscallback_http(struct lws *wsi,
                         enum lws_callback_reasons reason, void *user,
                         void *in, size_t len)
{
    struct lws_context *context = lws_get_context(wsi);
    const char *inPtr = (char*)in;
    char client_name[128], client_ip[128];
    const char *res = "";
#ifdef EXTERNAL_POLL
    int m;
    int fd = (int)(long)user;
#endif
    jsi_wsCmdObj *cmdPtr = (jsi_wsCmdObj *)lws_context_user(context);
    if (!cmdPtr) {
        fprintf(stderr, "bad ws context\n");
        return -1;
    }
    jsi_wsPss *pss = NULL;
    Jsi_Interp *interp = cmdPtr->interp;
    Jsi_Value* callPtr = NULL;
    int rc = 0, deflt = 0;
    WSSIGASSERT(cmdPtr, OBJ);
    if (Jsi_InterpGone(interp))
        cmdPtr->deleted = 1;
    if (cmdPtr->debug>=128)
        fprintf(stderr, "HTTP CALLBACK: len=%d, %p %d:%s\n", (int)len, user, reason, jsw_getReasonStr(reason));
    switch (reason) {
#ifndef EXTERNAL_POLL
    case LWS_CALLBACK_GET_THREAD_ID:
    case LWS_CALLBACK_UNLOCK_POLL:
    case LWS_CALLBACK_PROTOCOL_INIT:
    case LWS_CALLBACK_ADD_POLL_FD:
    case LWS_CALLBACK_DEL_POLL_FD:
    case LWS_CALLBACK_CHANGE_MODE_POLL_FD:
    case LWS_CALLBACK_LOCK_POLL:
        return rc;
#else
    case LWS_CALLBACK_ADD_POLL_FD:
        if (jsi_wsnum_pollfds >= max_poll_elements) {
            lwsl_err("LWS_CALLBACK_ADD_POLL_FD: too many sockets to track\n");
            return 1;
        }
        fd_lookup[fd] = jsi_wsnum_pollfds;
        jsi_wspollfds[jsi_wsnum_pollfds].fd = fd;
        jsi_wspollfds[jsi_wsnum_pollfds].events = (int)(long)len;
        jsi_wspollfds[jsi_wsnum_pollfds++].revents = 0;
        break;
    case LWS_CALLBACK_DEL_POLL_FD:
        if (!--jsi_wsnum_pollfds)
            break;
        m = fd_lookup[fd];
        jsi_wspollfds[m] = jsi_wspollfds[jsi_wsnum_pollfds];
        fd_lookup[jsi_wspollfds[jsi_wsnum_pollfds].fd] = m;
        break;
#endif
    default:
        deflt = 1;
        break;
    }
    if (deflt && cmdPtr->debug>16 && cmdPtr->debug<128) {
        fprintf(stderr, "HTTP CALLBACK: len=%d, %p %d:%s\n", (int)len, user, reason, jsw_getReasonStr(reason));
    }
    switch (reason) {
    case LWS_CALLBACK_WSI_DESTROY:
        break;
#if (LWS_LIBRARY_VERSION_MAJOR>1)
    case LWS_CALLBACK_RECEIVE_CLIENT_HTTP: {
        char buffer[1024 + LWS_PRE];
        char *px = buffer + LWS_PRE;
        int lenx = sizeof(buffer) - LWS_PRE;
        if (lws_http_client_read(wsi, &px, &lenx) < 0)
            return -1;
        break;
    }
    case LWS_CALLBACK_RECEIVE_CLIENT_HTTP_READ:
        if (jsi_wsrecv_callback(interp, cmdPtr, pss, inPtr, len, 0) != JSI_OK)
            rc = 1;
        break;
    case LWS_CALLBACK_COMPLETED_CLIENT_HTTP:
        if (jsi_wsrecv_callback(interp, cmdPtr, pss, inPtr, len, 1) != JSI_OK)
            rc = 1;
        break;
    case LWS_CALLBACK_CLIENT_APPEND_HANDSHAKE_HEADER:
        if (cmdPtr->post) {
            unsigned char **p = (unsigned char **)in, *end = (*p) + len;
            int n = 0;
            char buf[100];
            Jsi_ValueString(interp, cmdPtr->post, &n);
            snprintf(buf, sizeof(buf), "%d", n);
            if (lws_add_http_header_by_token(wsi,
                    WSI_TOKEN_HTTP_CONTENT_LENGTH,
                    (unsigned char *)buf, 2, p, end))
                return -1;
            if (lws_add_http_header_by_token(wsi,
                    WSI_TOKEN_HTTP_CONTENT_TYPE,
                    (unsigned char *)"application/x-www-form-urlencoded", 33, p, end))
                return -1;
            lws_client_http_body_pending(wsi, 1);
            lws_callback_on_writable(wsi);
        }
        break;
    case LWS_CALLBACK_CLIENT_HTTP_WRITEABLE: {
        int n = 0;
        char *cps = Jsi_ValueString(interp, cmdPtr->post, &n);
        char *buf = (char*)Jsi_Calloc(1, LWS_PRE + n + 1);
        Jsi_Strcpy(buf + LWS_PRE, cps);
        n = lws_write(wsi, (unsigned char *)&buf[LWS_PRE], strlen(&buf[LWS_PRE]), LWS_WRITE_HTTP);
        Jsi_Free(buf);
        if (n < 0)
            return -1;
        lws_client_http_body_pending(wsi, 0);
        break;
    }
#endif
    case LWS_CALLBACK_FILTER_NETWORK_CONNECTION:
        break;
    case LWS_CALLBACK_PROTOCOL_INIT:
        break;
    case LWS_CALLBACK_CLOSED_HTTP:
        if (cmdPtr->debug>2)
            fprintf(stderr, "CLOSED\n");
        if (!pss)
            pss = jsi_wsgetPss(cmdPtr, wsi, user, 0, 1);
        if (pss)
            jsi_wsdeletePss(pss);
        break;
    case LWS_CALLBACK_WSI_CREATE:
        break;
    case LWS_CALLBACK_CONFIRM_EXTENSION_OKAY:
        break;
    case LWS_CALLBACK_FILTER_HTTP_CONNECTION:
        if (cmdPtr->debug>1)
            fprintf(stderr, "FILTER CONNECTION: %s\n", inPtr);
        pss = jsi_wsgetPss(cmdPtr, wsi, user, 1, 1);
        Jsi_DSSet(&pss->url, inPtr);
        jsi_wsgetUriArgValue(interp, wsi, &pss->query, &pss->queryObj);
        if (cmdPtr->instCtx == context && (cmdPtr->clientName[0] || cmdPtr->clientIP[0])) {
            pss->clientName = cmdPtr->clientName;
            pss->clientIP = cmdPtr->clientIP;
        }
        Jsi_DSSetLength(&pss->dHdrs, 0);
        pss->hdrNum = jsi_wsGetHeaders(pss, wsi, &pss->dHdrs, pss->hdrSz, sizeof(pss->hdrSz)/sizeof(int));
        if (cmdPtr->onFilter && !cmdPtr->deleted) {
            int killcon = 0, n = 0;
            Jsi_Obj *oarg1;
            Jsi_Value *vpargs, *vargs[10], *ret = Jsi_ValueNew1(interp);
            vargs[n++] = Jsi_ValueNewObj(interp, cmdPtr->fobj);
            vargs[n++] = Jsi_ValueNewNumber(interp, (Jsi_Number)(pss->wid));
            vargs[n++] = Jsi_ValueNewBlob(interp, (uchar*)in, len);
            vargs[n++] = Jsi_ValueNewBoolean(interp, 1);
            vpargs = Jsi_ValueMakeObject(interp, NULL, oarg1 = Jsi_ObjNewArray(interp, vargs, n, 0));
            Jsi_IncrRefCount(interp, vpargs);
            Jsi_ValueMakeUndef(interp, &ret);
            rc = Jsi_FunctionInvoke(interp, cmdPtr->onFilter, vpargs, &ret, NULL);
            if (rc == JSI_OK && Jsi_ValueIsFalse(interp, ret)) {
                if (cmdPtr->debug>1)
                    fprintf(stderr, "WS:KILLING CONNECTION: %p\n", pss);
                killcon = 1;
            }
            Jsi_DecrRefCount(interp, vpargs);
            Jsi_DecrRefCount(interp, ret);
            if (rc != JSI_OK) {
                Jsi_LogError("websock bad rcv eval");
                return 1;
            }
            if (killcon)
                return 1;
        }
        break;
    case LWS_CALLBACK_SERVER_NEW_CLIENT_INSTANTIATED:
        client_name[0] = 0;
        client_ip[0] = 0;
        lws_get_peer_addresses(wsi, lws_get_socket_fd(wsi), client_name,
                                         sizeof(client_name), client_ip, sizeof(client_ip));
        if (client_name[0])
            cmdPtr->clientName = Jsi_KeyAdd(interp, client_name);
        if (client_ip[0])
            cmdPtr->clientIP = Jsi_KeyAdd(interp, client_ip);
        if (cmdPtr->clientName || cmdPtr->clientIP) {
            const char *loname = cmdPtr->localhostName;
            if (!loname) loname = "localhost";
            cmdPtr->instCtx = context;
            if (cmdPtr->debug>1)
                fprintf(stderr,  "Received network connect from %s (%s)\n",
                     cmdPtr->clientName, cmdPtr->clientIP);
#ifndef __WIN32
            if (cmdPtr->local && (cmdPtr->clientName && Jsi_Strcmp(cmdPtr->clientName, loname))) {
                if (cmdPtr->debug>1)
                    fprintf(stderr,  "Dropping non-localhost connection\n");
                return 1;
            }
#endif
        }
        if (cmdPtr->maxConnects && cmdPtr->stats.connectCnt>=cmdPtr->maxConnects) {
            if (cmdPtr->debug>1)
                fprintf(stderr, "maxConnects exceeded: rejecting connection <%p>\n", user);
            rc = -1;
        }
        break;
    case LWS_CALLBACK_HTTP:
    {
        Jsi_DString dStr;
        Jsi_DSInit(&dStr);
        if (cmdPtr->debug)
            fprintf(stderr, "HTTP GET: %s\n", inPtr);
        rc = jsi_wsHttp(interp, cmdPtr, wsi, user, context, inPtr, &dStr, pss);
        Jsi_DSFree(&dStr);
        if (rc<0)
            return -1;
        if (rc==1) {
            goto try_to_reuse;
        }
        break;
    }
#if (LWS_LIBRARY_VERSION_MAJOR>1)
    case LWS_CALLBACK_HTTP_BODY: {
        if (!pss)
            pss = jsi_wsgetPss(cmdPtr, wsi, user, 0, 1);
        if (!pss) break;
        callPtr = (pss->onUpload?pss->onUpload:cmdPtr->onUpload);
        if (cmdPtr->maxUpload<=0 || !callPtr) {
            if (cmdPtr->noWarn==0)
                fprintf(stderr, "Upload disabled: maxUpload=%d, onUpload=%p\n", cmdPtr->maxUpload, callPtr);
            return -1;
        }
        if (!pss->spa) {
            if (!pss->paramv) {
                if (cmdPtr->formParams && cmdPtr->formParams != jsi_wsparam_str)
                    Jsi_SplitStr(cmdPtr->formParams, &pss->paramc, &pss->paramv, ",", &pss->paramDS);
                else {
                    pss->paramv = (typeof(pss->paramv))jsi_wsparam_names;
                    pss->paramc = ARRAY_SIZE(jsi_wsparam_names);
                }
            }
            pss->spa = lws_spa_create(wsi, (const char*const*)pss->paramv,
                pss->paramc, 4096, jsi_wsFileUploadCB, pss);
            if (!pss->spa)
                    return -1;
            pss->filename[0] = '\0';
            pss->file_length = 0;
        }
        cmdPtr->stats.uploadLast = pss->stats.uploadLast = time(NULL);
        if (lws_spa_process(pss->spa, inPtr, len))
                return -1;
        if (!pss->stats.uploadStart) {
            cmdPtr->stats.uploadEnd = pss->stats.uploadEnd = 0;
            cmdPtr->stats.uploadStart = pss->stats.uploadStart = time(NULL);
            cmdPtr->stats.uploadCnt++;
            pss->stats.uploadCnt++;
        }
        break;
    }
    case LWS_CALLBACK_HTTP_BODY_COMPLETION:
        if (!pss) {
            pss = jsi_wsgetPss(cmdPtr, wsi, user, 0, 1);
            callPtr = (pss&&pss->onUpload?pss->onUpload:cmdPtr->onUpload);
        }
        if (pss && pss->spa)
            lws_spa_finalize(pss->spa);
        res = Jsi_DSValue(&pss->resultStr);
        if (!res[0]) {
            if (!pss->resultCode)
                res = "<html><body>Upload complete</body></html>";
            else
                res = "<html><body>Upload error</body></html>";
        }
        jsi_wsServeString(pss, wsi, res, pss->resultCode==JSI_OK?0:500, NULL, NULL);
        if (cmdPtr->maxUpload<=0 || !callPtr) {
            if (cmdPtr->noWarn==0)
                fprintf(stderr, "Upload disabled: maxUpload=%d, onUpload=%p\n", cmdPtr->maxUpload, callPtr);
            return -1;
        }
        cmdPtr->stats.uploadEnd = pss->stats.uploadEnd = time(NULL);
        lws_return_http_status(wsi, HTTP_STATUS_OK, NULL);
        goto try_to_reuse;
    case LWS_CALLBACK_HTTP_DROP_PROTOCOL:
        pss = jsi_wsgetPss(cmdPtr, wsi, user, 0, 1);
        if (pss && pss->spa) {
            lws_spa_destroy(pss->spa);
            pss->spa = NULL;
        }
        break;
#endif
    case LWS_CALLBACK_HTTP_FILE_COMPLETION:
        goto try_to_reuse;
    case LWS_CALLBACK_HTTP_WRITEABLE: {
        lwsl_info("LWS_CALLBACK_HTTP_WRITEABLE\n");
        if (!pss)
            pss = jsi_wsgetPss(cmdPtr, wsi, user, 0, 1);
        if (!pss || !pss->fop_fd)
            goto try_to_reuse;
        int sent = 0;
        unsigned char buffer[JSI_BUFSIZ*10 + LWS_PRE];
        do {
            int n = sizeof(buffer) - LWS_PRE;
            int m = lws_get_peer_write_allowance(wsi);
            if (m == 0)
                goto later;
            if (m != -1 && m < n)
                n = m;
            lws_filepos_t amount = 0;
            n = lws_vfs_file_read(pss->fop_fd, &amount, buffer + LWS_PRE, n);
            if (n < 0) {
                lwsl_err("problem reading file\n");
                goto bail;
            }
            n = (int)amount;
            if (n == 0)
                goto penultimate;
            m = jsi_wswrite(pss, wsi, buffer + LWS_PRE, n, LWS_WRITE_HTTP);
            if (m < 0) {
                lwsl_err("write failed\n");
                goto bail;
            }
            if (m)  
                lws_set_timeout(wsi, PENDING_TIMEOUT_HTTP_CONTENT, 5);
            sent += m;
        } while (!lws_send_pipe_choked(wsi) && (sent < 500 * 1024 * 1024));
later:
        lws_callback_on_writable(wsi);
        break;
penultimate:
        lws_vfs_file_close(&pss->fop_fd);
        goto try_to_reuse;
bail:
        lws_vfs_file_close(&pss->fop_fd);
        rc = -1;
        goto doret;
    }
    default:
        break;
    }
    goto doret;
try_to_reuse:
    if (lws_http_transaction_completed(wsi))
         rc = -1;
    else
        rc = 0;
    goto doret;
doret:
    if (cmdPtr->debug>2)
        fprintf(stderr, "<---HTTP RET = %d\n", rc);
    return rc;
}