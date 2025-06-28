jsi_wscallback_websock(struct lws *wsi,
      enum lws_callback_reasons reason,
      void *user, void *in, size_t len)
{
    struct lws_context *context = lws_get_context(wsi);
    jsi_wsPss *pss = NULL;
    jsi_wsCmdObj *cmdPtr = (jsi_wsCmdObj *)lws_context_user(context);
    if (!cmdPtr) {
        fprintf(stderr, "null ws context\n");
        return -1;
    }
    Jsi_Interp *interp = cmdPtr->interp;
    char *inPtr = (char*)in;
    int sLen, n, rc =0;
    WSSIGASSERT(cmdPtr, OBJ);
    if (Jsi_InterpGone(interp))
        cmdPtr->deleted = 1;
    if (cmdPtr->debug>=32) {
        switch (reason) {
            case LWS_CALLBACK_SERVER_WRITEABLE:
            case LWS_CALLBACK_CLIENT_WRITEABLE:
                break;
            default:
                fprintf(stderr, "WS CALLBACK: len=%d, %p %d:%s\n", (int)len, user, reason, jsw_getReasonStr(reason));
        }
    }
    switch (reason) {
    case LWS_CALLBACK_PROTOCOL_INIT:
        if (cmdPtr->noWebsock)
            return 1;
        break;
    case LWS_CALLBACK_FILTER_PROTOCOL_CONNECTION:
        pss = jsi_wsgetPss(cmdPtr, wsi, user, 1, 1);
        Jsi_DSSet(&pss->url, inPtr);
        if (cmdPtr->instCtx == context && (cmdPtr->clientName[0] || cmdPtr->clientIP[0])) {
            pss->clientName = cmdPtr->clientName;
            pss->clientIP = cmdPtr->clientIP;
        }
        if (cmdPtr->onFilter && !cmdPtr->deleted) {
            if (!pss)
                pss = jsi_wsgetPss(cmdPtr, wsi, user, 1, 0);
            int killcon = 0, n = 0;
            Jsi_Obj *oarg1;
            Jsi_Value *vpargs, *vargs[10], *ret = Jsi_ValueNew1(interp);
            vargs[n++] = Jsi_ValueNewObj(interp, cmdPtr->fobj);
            vargs[n++] = Jsi_ValueNewNumber(interp, (Jsi_Number)(pss->wid));
            vargs[n++] = Jsi_ValueNewBlob(interp, (uchar*)in, len);
            vargs[n++] = Jsi_ValueNewBoolean(interp, 0);
            vpargs = Jsi_ValueMakeObject(interp, NULL, oarg1 = Jsi_ObjNewArray(interp, vargs, n, 0));
            Jsi_IncrRefCount(interp, vpargs);
            Jsi_ValueMakeUndef(interp, &ret);
            rc = Jsi_FunctionInvoke(interp, cmdPtr->onFilter, vpargs, &ret, NULL);
            if (rc == JSI_OK && Jsi_ValueIsFalse(interp, ret)) {
                if (cmdPtr->debug>1)
                    fprintf(stderr, "WS:KILLING CONNECTION: %p\n", user);
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
    case LWS_CALLBACK_CLIENT_ESTABLISHED:
    case LWS_CALLBACK_ESTABLISHED:
        if (cmdPtr->bufferPwr2>0) {
            char nbuf[100];
            snprintf(nbuf, sizeof(nbuf), "%d", cmdPtr->bufferPwr2);
            lws_set_extension_option(wsi, "permessage-deflate", "rx_buf_size", nbuf);
            lws_set_extension_option(wsi, "permessage-deflate", "tx_buf_size", nbuf);
        }
        if (!pss)
            pss = jsi_wsgetPss(cmdPtr, wsi, user, 1, 0);
        if (cmdPtr->onOpen && !cmdPtr->deleted) {
            Jsi_Obj *oarg1;
            Jsi_Value *vpargs, *vargs[10];
            int n = 0;
            vargs[n++] = Jsi_ValueNewObj(interp, cmdPtr->fobj);
            vargs[n++] = Jsi_ValueNewNumber(interp, (Jsi_Number)(pss->wid));
            vpargs = Jsi_ValueMakeObject(interp, NULL, oarg1 = Jsi_ObjNewArray(interp, vargs, n, 0));
            Jsi_IncrRefCount(interp, vpargs);
            Jsi_Value *ret = Jsi_ValueNew1(interp);
            Jsi_ValueMakeUndef(interp, &ret);
            rc = Jsi_FunctionInvoke(interp, cmdPtr->onOpen, vpargs, &ret, NULL);
            Jsi_DecrRefCount(interp, vpargs);
            Jsi_DecrRefCount(interp, ret);
            if (rc != JSI_OK)
                return Jsi_LogError("websock bad rcv eval");
        }
        break;
    case LWS_CALLBACK_WSI_DESTROY:
        break;
    case LWS_CALLBACK_CLOSED:
    case LWS_CALLBACK_PROTOCOL_DESTROY:
        pss = jsi_wsgetPss(cmdPtr, wsi, user, 0, 0);
        if (!pss) break;
        if (cmdPtr->onClose || pss->onClose) {
            rc = jsi_wsrecv_callback(interp, cmdPtr, pss, inPtr, len, 1);
            if (rc != JSI_OK)
                return Jsi_LogError("websock bad rcv eval");
        }
        jsi_wsdeletePss(pss);
        if (cmdPtr->stats.connectCnt<=0 && cmdPtr->onCloseLast && !Jsi_InterpGone(interp)) {
            Jsi_RC jrc;
            Jsi_Value *retStr = Jsi_ValueNew1(interp);
            Jsi_Value *vpargs, *vargs[10];
            int n = 0;
            vargs[n++] = (cmdPtr->deleted?Jsi_ValueNewNull(interp):Jsi_ValueNewObj(interp, cmdPtr->fobj));
            vpargs = Jsi_ValueMakeObject(interp, NULL, Jsi_ObjNewArray(interp, vargs, n, 0));
            Jsi_IncrRefCount(interp, vpargs);
            jrc = Jsi_FunctionInvoke(interp, cmdPtr->onCloseLast, vpargs, &retStr, NULL);
            Jsi_DecrRefCount(interp, vpargs);
            Jsi_DecrRefCount(interp, retStr);
            if (Jsi_InterpGone(interp))
                return JSI_ERROR;
            return jrc;
        }
        break;
    case LWS_CALLBACK_CLIENT_WRITEABLE:
    case LWS_CALLBACK_SERVER_WRITEABLE: {
        pss = jsi_wsgetPss(cmdPtr, wsi, user, 0, 0);
        if (!pss || !pss->stack) break;
        if (pss->lastData)
            Jsi_Free(pss->lastData);
        n=0;
        char *data = pss->lastData = (char*)Jsi_StackUnshift(pss->stack);
        unsigned char *p;
        if (data == NULL)
            break;
        pss->stats.msgQLen--;
        pss->state = PWS_SENT;
        p = (unsigned char *)data+LWS_PRE;
        sLen = Jsi_Strlen((char*)p);
        n = jsi_wswrite(pss, wsi, p, sLen, (pss->stats.isBinary?LWS_WRITE_BINARY:LWS_WRITE_TEXT));
        if (cmdPtr->debug>=10)
            fprintf(stderr, "WS:CLIENT WRITE(%p): %d=>%d\n", pss, sLen, n);
        if (n >= 0) {
            cmdPtr->stats.sentCnt++;
            cmdPtr->stats.sentLast = time(NULL);
            pss->stats.sentCnt++;
            pss->stats.sentLast = time(NULL);
        } else {
            lwsl_err("ERROR %d writing to socket\n", n);
            pss->state = PWS_SENDERR;
            pss->stats.sentErrCnt++;
            pss->stats.sentErrLast = time(NULL);
            cmdPtr->stats.sentErrCnt++;
            cmdPtr->stats.sentErrLast = time(NULL);
            rc = 1;
        }
        break;
    }
    case LWS_CALLBACK_CLIENT_RECEIVE:
    case LWS_CALLBACK_RECEIVE:
    {
        pss = jsi_wsgetPss(cmdPtr, wsi, user, 0, 0);
        if (!pss) break;
        pss->stats.recvCnt++;
        pss->stats.recvLast = time(NULL);
        cmdPtr->stats.recvCnt++;
        cmdPtr->stats.recvLast = time(NULL);
        if (cmdPtr->onRecv || pss->onRecv) {
            int nlen = len;
            if (nlen<=0)
                return 0;
            int rblen = Jsi_DSLength(&pss->recvBuf),
                bmax = cmdPtr->recvBufMax,
                isfin = pss->stats.isFinal = lws_is_final_fragment(wsi);
            pss->stats.isBinary = lws_frame_is_binary(wsi);
            if (rblen) {
                if (bmax && rblen>bmax) {
                    fprintf(stderr, "WS: Recv exceeds recvBufMax: %d>%d\n", rblen, bmax);
                    rc = 1;
                    break;
                }
                Jsi_DSAppendLen(&pss->recvBuf, inPtr, len);
                if (!isfin) break;
                cmdPtr->recvBufCnt--;
                nlen = Jsi_DSLength(&pss->recvBuf);
                inPtr = Jsi_DSFreeDup(&pss->recvBuf);
            } else {
                if (!isfin) {
                    cmdPtr->recvBufCnt++;
                    Jsi_DSAppendLen(&pss->recvBuf, inPtr, len);
                    break;
                }
            }
            rc = jsi_wsrecv_callback(interp, cmdPtr, pss, inPtr, nlen, 0);
            if (inPtr != in)
                Jsi_Free(inPtr);
            if (rc != JSI_OK) {
                Jsi_LogError("websock bad rcv eval");
                return 1;
            }
        }
        lws_callback_on_writable_all_protocol(cmdPtr->context, lws_get_protocol(wsi));
        break;
    }
    default:
        break;
    }
    return rc;
}