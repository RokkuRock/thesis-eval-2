SockParse(Sock *sockPtr)
{
    const Tcl_DString  *bufPtr;
    const Driver       *drvPtr;
    Request            *reqPtr;
    char                save;
    SockState           result;
    NS_NONNULL_ASSERT(sockPtr != NULL);
    drvPtr = sockPtr->drvPtr;
    NsUpdateProgress((Ns_Sock *) sockPtr);
    reqPtr = sockPtr->reqPtr;
    bufPtr = &reqPtr->buffer;
    while (reqPtr->coff == 0u) {
        char *s, *e;
        size_t cnt;
        s = bufPtr->string + reqPtr->roff;
        e = memchr(s, INTCHAR('\n'), reqPtr->avail);
        if (unlikely(e == NULL)) {
            return SOCK_MORE;
        }
        if (unlikely((e - s) > drvPtr->maxline)) {
            sockPtr->keep = NS_FALSE;
            if (reqPtr->request.line == NULL) {
                Ns_Log(DriverDebug, "SockParse: maxline reached of %d bytes",
                       drvPtr->maxline);
                sockPtr->flags = NS_CONN_REQUESTURITOOLONG;
                Ns_Log(Warning, "request line is too long (%d bytes)", (int)(e - s));
            } else {
                sockPtr->flags = NS_CONN_LINETOOLONG;
                Ns_Log(Warning, "request header line is too long (%d bytes)", (int)(e - s));
            }
        }
        cnt = (size_t)(e - s) + 1u;
        reqPtr->roff  += cnt;
        reqPtr->avail -= cnt;
        if (likely(e > s) && likely(*(e-1) == '\r')) {
            --e;
        }
        if (unlikely(e == s) && (reqPtr->coff == 0u)) {
            reqPtr->coff = EndOfHeader(sockPtr);
            if ((sockPtr->flags & NS_CONN_CONTINUE) != 0u) {
                Ns_Log(Ns_LogRequestDebug, "honoring 100-continue");
                if ((sockPtr->flags & NS_CONN_ENTITYTOOLARGE) != 0u) {
                    Ns_Log(Ns_LogRequestDebug, "100-continue: entity too large");
                    return SOCK_ENTITYTOOLARGE;
                } else {
                    struct iovec iov[1];
                    ssize_t      sent;
                    Ns_Log(Ns_LogRequestDebug, "100-continue: reply CONTINUE");
                    iov[0].iov_base = (char *)"HTTP/1.1 100 Continue\r\n\r\n";
                    iov[0].iov_len = strlen(iov[0].iov_base);
                    sent = Ns_SockSendBufs((Ns_Sock *)sockPtr, iov, 1,
                                           NULL, 0u);
                    if (sent != (ssize_t)iov[0].iov_len) {
                        Ns_Log(Warning, "could not deliver response: 100 Continue");
                    }
                }
            }
        } else {
            save = *e;
            *e = '\0';
            if (unlikely(reqPtr->request.line == NULL)) {
                Ns_Log(DriverDebug, "SockParse (%d): parse request line <%s>", sockPtr->sock, s);
                if (Ns_ParseRequest(&reqPtr->request, s) == NS_ERROR) {
                    return SOCK_BADREQUEST;
                }
                if (unlikely(reqPtr->request.version < 1.0)) {
                    reqPtr->coff = reqPtr->roff;
                    Ns_Log(Notice, "pre-HTTP/1.0 request <%s>", reqPtr->request.line);
                }
            } else if (Ns_ParseHeader(reqPtr->headers, s, Preserve) != NS_OK) {
                return SOCK_BADHEADER;
            } else {
                if (unlikely(Ns_SetSize(reqPtr->headers) > (size_t)drvPtr->maxheaders)) {
                    Ns_Log(DriverDebug, "SockParse (%d): maxheaders reached of %d bytes",
                           sockPtr->sock, drvPtr->maxheaders);
                    return SOCK_TOOMANYHEADERS;
                }
            }
            *e = save;
        }
    }
    if (unlikely(reqPtr->request.line == NULL)) {
        return SOCK_BADREQUEST;
    }
    assert(reqPtr->coff > 0u);
    assert(reqPtr->request.line != NULL);
    Ns_Log(Dev, "=== length < avail (length %" PRIuz
           ", avail %" PRIuz ") tfd %d tfile %p chunkStartOff %" PRIuz,
           reqPtr->length, reqPtr->avail, sockPtr->tfd,
           (void *)sockPtr->tfile, reqPtr->chunkStartOff);
    if (reqPtr->chunkStartOff != 0u) {
        bool   complete;
        size_t currentContentLength;
        complete = ChunkedDecode(reqPtr, NS_TRUE);
        currentContentLength = reqPtr->chunkWriteOff - reqPtr->coff;
        if ((!complete)
            || (reqPtr->expectedLength != 0u && currentContentLength < reqPtr->expectedLength)) {
            return SOCK_MORE;
        }
        reqPtr->length = (size_t)currentContentLength;
    }
    if (reqPtr->avail < reqPtr->length) {
        Ns_Log(DriverDebug, "SockRead wait for more input");
        return SOCK_MORE;
    }
    Ns_Log(Dev, "=== all required data is available (avail %" PRIuz", length %" PRIuz ", "
           "readahead %" TCL_LL_MODIFIER "d maxupload %" TCL_LL_MODIFIER "d) tfd %d",
           reqPtr->avail, reqPtr->length, drvPtr->readahead, drvPtr->maxupload,
           sockPtr->tfd);
    result = SOCK_READY;
    if (sockPtr->tfile != NULL) {
        reqPtr->content = NULL;
        reqPtr->next = NULL;
        reqPtr->avail = 0u;
        Ns_Log(DriverDebug, "content spooled to file: size %" PRIdz ", file %s",
               reqPtr->length, sockPtr->tfile);
    } else {
        if (sockPtr->tfd > 0) {
#ifdef _WIN32
            assert(0);
#else
            int prot = PROT_READ | PROT_WRITE;
            ssize_t rc = ns_write(sockPtr->tfd, "\0", 1);
            if (rc == -1) {
                Ns_Log(Error, "socket: could not append terminating 0-byte");
            }
            sockPtr->tsize = reqPtr->length + 1;
            sockPtr->taddr = mmap(0, sockPtr->tsize, prot, MAP_PRIVATE,
                                  sockPtr->tfd, 0);
            if (sockPtr->taddr == MAP_FAILED) {
                sockPtr->taddr = NULL;
                result = SOCK_ERROR;
            } else {
                reqPtr->content = sockPtr->taddr;
                Ns_Log(Debug, "content spooled to mmapped file: readahead=%"
                       TCL_LL_MODIFIER "d, filesize=%" PRIdz,
                       drvPtr->readahead, sockPtr->tsize);
            }
#endif
        } else {
            reqPtr->content = bufPtr->string + reqPtr->coff;
        }
        reqPtr->next = reqPtr->content;
        if (reqPtr->length > 0u) {
            Ns_Log(DriverDebug, "SockRead adds null terminating character at content[%" PRIuz "]", reqPtr->length);
            reqPtr->savedChar = reqPtr->content[reqPtr->length];
            reqPtr->content[reqPtr->length] = '\0';
            if (sockPtr->taddr == NULL) {
                LogBuffer(DriverDebug, "UPDATED BUFFER", sockPtr->reqPtr->buffer.string, (size_t)reqPtr->buffer.length);
            }
        }
    }
    return result;
}