static int jsi_wsHttp(Jsi_Interp *interp, jsi_wsCmdObj *cmdPtr, struct lws *wsi, void *user,
    struct lws_context *context, const char* inPtr, Jsi_DString *tStr, jsi_wsPss *pss)
{
    const char *ext = NULL;
    unsigned char buffer[JSI_BUFSIZ];
    const char *mime = NULL;
    time_t now = time(NULL);
    char buf[JSI_BUFSIZ];
    int rc = 0;
    buf[0] = 0;
    uchar *p = buffer, *end = &buffer[sizeof(buffer)-1];
    int n;
    Jsi_Value* fname = NULL;
    bool isJsiWeb = 0, isSSI = 0;
    cmdPtr->stats.httpLast = now;
    if (lws_hdr_total_length(wsi, WSI_TOKEN_POST_URI))
        return 0;
    if (!pss)
        pss = jsi_wsgetPss(cmdPtr, wsi, user, 1, 1);
    int uplen=(cmdPtr->urlPrefix?Jsi_Strlen(cmdPtr->urlPrefix):0);
    if (inPtr && cmdPtr->urlPrefix && !Jsi_Strncmp(inPtr, cmdPtr->urlPrefix, uplen))
        inPtr += uplen;
    if (cmdPtr->redirDisable) { 
        if (difftime(now, cmdPtr->stats.redirLast)>=600)
            cmdPtr->redirDisable = 0;
        else
            cmdPtr->redirDisable--;
    }
    if ((cmdPtr->urlRedirect && (inPtr == 0 || *inPtr == 0 || !Jsi_Strcmp(inPtr, "/")) && !cmdPtr->redirDisable)
        && (inPtr = cmdPtr->urlRedirect) && inPtr[0]) {
        cmdPtr->stats.redirCnt++;
        if (cmdPtr->redirMax>0 && !cmdPtr->redirDisable && cmdPtr->redirMax>0 && cmdPtr->stats.redirLast
            && difftime(now, cmdPtr->stats.redirLast)<600 && ++cmdPtr->redirAllCnt>cmdPtr->redirMax)
            cmdPtr->redirDisable = 100;
        cmdPtr->stats.redirLast = now;
        rc = lws_http_redirect(wsi, 301, (uchar*)inPtr, Jsi_Strlen(inPtr), &p, end);
        return (rc == 100 ? 0 : 1);
    }
    if (!inPtr || !*inPtr)
        inPtr = "/";
    if (cmdPtr->useridPass || cmdPtr->onAuth) {
        int ok = 0;
        int alen;
        const char *auth = jsi_wsHeader(pss, "authorization", &alen);
        if (auth && !Jsi_Strncasecmp(auth, "basic ", 6) && !cmdPtr->deleted) {
            auth += 6;
            Jsi_DString eStr = {}, bStr = {};
            Jsi_DSAppendLen(&eStr, auth, alen - 6);
            Jsi_Base64(Jsi_DSValue(&eStr), -1, &bStr, 1);
            const char *bp = Jsi_DSValue(&bStr);
            if (bp && bp[0]) {
                if (!cmdPtr->onAuth)
                    ok = (!Jsi_Strcmp(cmdPtr->useridPass, bp));
                else {
                    Jsi_Obj *oarg1;
                    Jsi_Value *vpargs, *vargs[10];
                    int n = 0;
                    vargs[n++] = Jsi_ValueNewObj(interp, cmdPtr->fobj);
                    vargs[n++] = Jsi_ValueNewNumber(interp, (Jsi_Number)(pss->wid));
                    vargs[n++] = Jsi_ValueNewStringDup(interp, inPtr);
                    vargs[n++] = Jsi_ValueNewStringDup(interp, bp);
                    vpargs = Jsi_ValueMakeObject(interp, NULL, oarg1 = Jsi_ObjNewArray(interp, vargs, n, 0));
                    Jsi_IncrRefCount(interp, vpargs);
                    Jsi_Value *ret = Jsi_ValueNew1(interp);
                    bool rb = 0;
                    rc = Jsi_FunctionInvoke(interp, cmdPtr->onAuth, vpargs, &ret, NULL);
                    if (rc == JSI_OK)
                        rb = !Jsi_ValueIsFalse(interp, ret);
                    Jsi_DecrRefCount(interp, vpargs);
                    Jsi_DecrRefCount(interp, ret);
                    if (rc != JSI_OK) {
                        Jsi_LogError("websock bad rcv eval");
                        return -1;
                    }
                    ok = rb;
                }
            }
            Jsi_DSFree(&eStr);
            Jsi_DSFree(&bStr);
        }
        if (!ok) {
            const char *realm = (cmdPtr->realm?cmdPtr->realm:"jsish");
            int n = snprintf(buf, sizeof(buf), "Basic realm=\"%s\"", realm);
            if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_WWW_AUTHENTICATE,
                    (unsigned char *)buf, n, &p, end))
                return -1;
            if (jsi_wsServeString(pss, wsi, "Password is required to access this page", 401, (char*)buffer, NULL)<0)
                return -1;
            return lws_http_transaction_completed(wsi);
        }
    }
    if (cmdPtr->onGet || pss->onGet) {
        Jsi_RC jrc;
        int rrv = 1;
        if (cmdPtr->getRegexp) {
            rrv = 0;
            jrc = Jsi_RegExpMatch(interp, cmdPtr->getRegexp, inPtr, &rrv, NULL);
            if (jrc != JSI_OK)
                return -1;  
        }
        if (rrv) {
            jrc = jsi_wsGetCmd(interp, cmdPtr, pss, wsi, inPtr, pss->onGet?pss->onGet:cmdPtr->onGet, tStr);
            switch (jrc) {
                case JSI_ERROR: return -1;
                case JSI_OK: return 0;
                case JSI_SIGNAL:
                    return jsi_ws_http_redirect(wsi, 302, tStr, &p, end);
                case JSI_CONTINUE:
                    inPtr = Jsi_DSValue(tStr); break;
                case JSI_BREAK: break;
                default: break;
            }
        }
    }
    ext = Jsi_Strrchr(inPtr, '.');
    Jsi_Value *rdir = (pss->rootdir?pss->rootdir:cmdPtr->rootdir);
    cmdPtr->curRoot = (rdir?Jsi_ValueString(cmdPtr->interp, rdir, NULL):"./");
    Jsi_DString sStr;
    Jsi_DSInit(&sStr);
    jsi_wsPathAlias(interp, cmdPtr, &inPtr, &sStr);
    snprintf(buf, sizeof(buf), "%s/%s", cmdPtr->curRoot, inPtr);
    Jsi_DSFree(&sStr);
    if (cmdPtr->debug>1)
        fprintf(stderr, "FILE: %s in %s | %s\n", buf, cmdPtr->curRoot, Jsi_ValueString(interp, cmdPtr->rootdir, NULL));
    char extBuf[100], *cpde = Jsi_Strrchr(buf, '/');
    isJsiWeb = (cpde && cmdPtr->jsiFnPattern && Jsi_GlobMatch(cmdPtr->jsiFnPattern, cpde+1, 0));
    bool isgzip = 0;
    if (!ext || !ext[1])
        mime = "text/html";
    else {
        const char *eext = ext+1;
        uint elen = Jsi_Strlen(ext);
        if (elen>3 && elen<(sizeof(extBuf)-10) && !Jsi_Strcmp(ext+elen-3,".gz")) {
            Jsi_Strcpy(extBuf, ext);
            extBuf[elen-3] = 0;
            char *ext2 = Jsi_Strrchr(extBuf, '.');
            if (ext2) {
                isgzip = 1;
                ext = ext2;
            }
        }
        Jsi_HashEntry *hPtr;
        if (cmdPtr->mimeTypes) {
            Jsi_Value *mVal = Jsi_ValueObjLookup(interp, cmdPtr->mimeTypes, ext+1, 1);
            if (mVal)
                mime = Jsi_ValueString(interp, mVal, NULL);
        }
        if (!mime) {
            static const char* mtypes[] = {
                "html", "text/html", "js", "application/x-javascript",
                "css", "text/css", "png", "image/png", "ico", "image/icon",
                "gif", "image/gif", "jpeg", "image/jpeg",
                "jpg", "image/jpeg", "svg", "image/svg+xml",
                "json", "application/json", "txt", "text/plain",
                "jsi", "application/x-javascript", "cssi", "text/css",
                "shtml", "text/html",  "scss", "text/css",
                "sjs", "application/x-javascript",
                0, 0
            };
            mime = "text/html";
            int i;
            for (i=0; mtypes[i]; i+=2)
                if (tolower(*eext) == mtypes[i][0] && !Jsi_Strncasecmp(eext, mtypes[i], -1)) {
                    mime = mtypes[i+1];
                    break;
                }
        }
        isSSI = jsi_wsIsSSIExt(interp, cmdPtr, pss, eext);
        if ((hPtr = Jsi_HashEntryFind(cmdPtr->handlers, ext)) && !cmdPtr->deleted) {
            Jsi_DString jStr = {};
            Jsi_Value *vrc = NULL;
            int hrc = 0, strLen, evrc, isalloc=0;
            char *vStr, *hstr = NULL;
            jsi_wsHander *hdlPtr = (jsi_wsHander*)Jsi_HashValueGet(hPtr);
            Jsi_Value *hv = hdlPtr->val;
            if (Jsi_Strchr(buf, '\'') || Jsi_Strchr(buf, '\"')) {
                jsi_wsServeString(pss, wsi, "Can not handle quotes in url", 404, NULL, NULL);
                return -1;
            }
            cmdPtr->handlersPkg=1;
            if ((hdlPtr->flags&1) && cmdPtr->handlersPkg && Jsi_ValueIsString(interp, hv)
                && ((hstr = Jsi_ValueString(interp, hv, NULL)))) {
                vrc = Jsi_NameLookup(interp, hstr);
                if (!vrc) {
                    Jsi_Number pver = Jsi_PkgRequire(interp, hstr, 0);
                    if (pver >= 0)
                        vrc = Jsi_NameLookup(interp, hstr);
                }
                if (!vrc || !Jsi_ValueIsFunction(interp, vrc)) {
                    if (vrc)
                        Jsi_DecrRefCount(interp, vrc);
                    Jsi_LogError("Failed to autoload handle: %s", hstr);
                    jsi_wsServeString(pss, wsi, "Failed to autoload handler", 404, NULL, NULL);
                    return -1;
                }
                if (hdlPtr->val)
                    Jsi_DecrRefCount(interp, hdlPtr->val);
                hdlPtr->val = vrc;
                Jsi_IncrRefCount(interp, vrc);
                hv = vrc;
            }
            if ((hdlPtr->flags&2) && !hdlPtr->triedLoad && !hdlPtr->objVar && Jsi_ValueIsFunction(interp, hv)) {
                hdlPtr->triedLoad = 1;
                Jsi_DSAppend(&jStr, "[null]", NULL);
                Jsi_DSAppend(&jStr, "]", NULL);
                vrc = Jsi_ValueNew1(interp);
                evrc = Jsi_FunctionInvokeJSON(interp, hv, Jsi_DSValue(&jStr), &vrc);
                if (Jsi_InterpGone(interp))
                    return -1;
                if (evrc != JSI_OK || !vrc || !Jsi_ValueIsObjType(interp, vrc, JSI_OT_OBJECT)) {
                    Jsi_LogError("Failed to load obj: %s", hstr);
                    jsi_wsServeString(pss, wsi, "Failed to load obj", 404, NULL, NULL);
                    return -1;
                }
                Jsi_Value *fvrc = Jsi_ValueObjLookup(interp, vrc, "parse", 0);
                if (!fvrc || !Jsi_ValueIsFunction(interp, fvrc)) {
                    Jsi_LogError("Failed to find parse: %s", hstr);
                    jsi_wsServeString(pss, wsi, "Failed to find parse", 404, NULL, NULL);
                    return -1;
                }
                hdlPtr->objVar = fvrc;
                Jsi_IncrRefCount(interp, fvrc);
                hv = vrc;
            }
            if (hdlPtr->objVar) {   
                Jsi_DSAppend(&jStr, "[\"", buf, "\"]", NULL);  
                vrc = Jsi_ValueNew1(interp);
                evrc = Jsi_FunctionInvokeJSON(interp, hdlPtr->objVar, Jsi_DSValue(&jStr), &vrc);
                isalloc = 1;
            }
            else if (Jsi_ValueIsFunction(interp, hv)) {
                Jsi_DSAppend(&jStr, "[\"", buf, "\", {wsName:\"", Jsi_DSValue(&cmdPtr->cName), "\"", "}]", NULL);  
                vrc = Jsi_ValueNew1(interp);
                evrc = Jsi_FunctionInvokeJSON(interp, hv, Jsi_DSValue(&jStr), &vrc);
                isalloc = 1;
            } else {
                hstr = Jsi_ValueString(interp, hv, NULL);
                Jsi_DSAppend(&jStr, hstr, "('", buf, "');", NULL);
                evrc = Jsi_EvalString(interp, Jsi_DSValue(&jStr), JSI_EVAL_RETURN);
                if (evrc == JSI_OK)
                    vrc = Jsi_InterpResult(interp);
            }
            if (evrc != JSI_OK) {
                Jsi_LogError("failure in websocket handler");
            } else if ((!vrc) ||
                (!(vStr = Jsi_ValueString(interp, vrc, &strLen)))) {
                Jsi_LogError("failed to get result");
            } else {
                hrc = jsi_wsServeString(pss, wsi, vStr, 0, NULL, mime);
            }
            Jsi_DSFree(&jStr);
            if (isalloc)
                Jsi_DecrRefCount(interp, vrc);
            if (hrc<=0)
                return -1;
            return 1;
        }
    }
    if (!buf[0]) {
        if (cmdPtr->debug)
            fprintf(stderr, "empty file: %s\n", inPtr);
        return -1;
    }
    fname = Jsi_ValueNewStringDup(interp, buf);
    Jsi_IncrRefCount(interp, fname);
    Jsi_DString hStr = {};
    Jsi_StatBuf jsb;
    bool native = Jsi_FSNative(interp, fname);
    if ((native && Jsi_InterpSafe(interp) && Jsi_InterpAccess(interp, fname, JSI_INTACCESS_READ) != JSI_OK) ||
        (Jsi_Stat(interp, fname, &jsb) || jsb.st_size<=0)) {
nofile:
        if (cmdPtr->onUnknown || pss->onUnknown) {
            Jsi_Value *uk = (pss->onUnknown?pss->onUnknown:cmdPtr->onUnknown);
            Jsi_RC jrc = jsi_wsGetCmd(interp, cmdPtr, pss, wsi, inPtr, uk, NULL);
            if (jrc == JSI_ERROR)
                goto bail;
            if (jrc == JSI_OK)
                goto done;
        }
        if (0 && Jsi_Strstr(buf, "favicon.ico"))
            rc = jsi_wsServeString(pss, wsi, "data:;base64,iVBORw0KGgo=", 200, NULL, "image/icon");
        else {
            const char *cp = Jsi_Strrchr(buf,'/');
            if (cp && cp[1]) {
                char statPath[PATH_MAX];
                snprintf(statPath, sizeof(statPath), "/zvfs/lib/web%s", cp);
                Jsi_DecrRefCount(interp, fname);
                fname = Jsi_ValueNewStringDup(interp, statPath);
                Jsi_IncrRefCount(interp, fname);
                if (!Jsi_Stat(interp, fname, &jsb) && jsb.st_size>0) {
                    native = 0;
                    goto serve;
                }
            }
            if (cmdPtr->noWarn==0 && !Jsi_Strstr(buf, "favicon.ico"))
                fprintf(stderr, "failed open file for read: %s\n", buf);
            rc = jsi_wsServeString(pss, wsi, "<b style='color:red'>ERROR: can not serve file!</b>", 404, NULL, NULL);
        }
        Jsi_DecrRefCount(interp, fname);
        goto done;
    }
    if (!ext || isSSI)
        goto serve;
    if (S_ISDIR(jsb.st_mode)) {
        if (cmdPtr->noWarn==0)
            fprintf(stderr, "can not serve directory: %s\n", buf);
        rc = jsi_wsServeString(pss, wsi, "<b style='color:red'>ERROR: can not serve directory!</b>", 404, NULL, NULL);
        Jsi_DecrRefCount(interp, fname);
        goto done;
    }
serve:
    n = 0;
    static const char stsStr[] = "max-age=15768000 ; includeSubDomains";
    if (lws_is_ssl(wsi) && lws_add_http_header_by_name(wsi,
                    (uchar *) "Strict-Transport-Security:",
                    (uchar *) stsStr,
                    sizeof(stsStr)-1, &p, (uchar *)buffer + sizeof(buffer)))
        goto bail;
    n = p - buffer;
    if (n>0)
        Jsi_DSAppendLen(&hStr, (char*)buffer, n);
    p = buffer;
    if (isgzip) {
        if (lws_add_http_header_by_token(wsi, WSI_TOKEN_HTTP_CONTENT_ENCODING,
                    (unsigned char *)"gzip", n, &p, end))
            goto bail;
    }
    if (cmdPtr->headers && !jsi_wsAddHeader(interp, cmdPtr, wsi, cmdPtr->headers, &hStr))
        goto bail;
    if (pss->headers && !jsi_wsAddHeader(interp, cmdPtr, wsi, pss->headers, &hStr))
        goto bail;
    n = Jsi_DSLength(&hStr);
    if (native && !isSSI && !isJsiWeb) {
        if (!jsi_wsAddStdHeader(interp, cmdPtr, wsi, &hStr)) {
            Jsi_DecrRefCount(interp, fname);
            goto bail;
        }
        int hrc = lws_serve_http_file(wsi, buf, mime, Jsi_DSValue(&hStr), Jsi_DSLength(&hStr));
        if (hrc >= 0 && cmdPtr->onModify)
            jsi_wsFileAdd(interp, cmdPtr, fname);
        Jsi_DecrRefCount(interp, fname);
        if (hrc<0) {
            if (cmdPtr->noWarn==0)
                fprintf(stderr, "can not serve file (%d): %s\n", hrc, buf);
            goto bail;
        } else if (hrc > 0 && lws_http_transaction_completed(wsi))
            goto bail;
    } else {
        Jsi_DString dStr = {}, fStr = {};
        if (isSSI)
            rc = jsi_wsEvalSSI(interp, cmdPtr, fname, &fStr, 1, pss);
        else {
            rc = jsi_wsFileRead(interp, fname, &fStr, cmdPtr, pss);
            if (isJsiWeb)
                Jsi_DSAppend(&fStr, "\nwindow.jsiWebSocket=true;", NULL);
        }
        if (rc != JSI_OK) {
            Jsi_DSFree(&fStr);
            goto nofile;
        }
        int hrc = jsi_wsServeHeader(pss, wsi, (int)Jsi_DSLength(&fStr), 200, Jsi_DSValue(&hStr), mime, &dStr);
        if (hrc>=0) {
            Jsi_DSAppendLen(&dStr, Jsi_DSValue(&fStr), Jsi_DSLength(&fStr));
            char *strVal = Jsi_DSValue(&dStr);
            int strLen = Jsi_DSLength(&dStr);
            hrc = jsi_wswrite(pss, wsi, (unsigned char*)strVal, strLen, LWS_WRITE_HTTP);
        }
        Jsi_DecrRefCount(interp, fname);
        Jsi_DSFree(&dStr);
        Jsi_DSFree(&fStr);
        if (hrc<0) {
            if (cmdPtr->noWarn==0)
                fprintf(stderr, "can not serve data (%d): %s\n", hrc, buf);
            goto bail;
        } else if (hrc > 0 && lws_http_transaction_completed(wsi))
            goto bail;
    }
done:
    Jsi_DSFree(&hStr);
    return rc;
bail:
    rc = 1;
    goto done;
}