webSocketsDecodeHybi(rfbClientPtr cl, char *dst, int len)
{
    char *buf, *payload;
    uint32_t *payload32;
    int ret = -1, result = -1;
    int total = 0;
    ws_mask_t mask;
    ws_header_t *header;
    int i;
    unsigned char opcode;
    ws_ctx_t *wsctx = (ws_ctx_t *)cl->wsctx;
    int flength, fhlen;
    if (wsctx->readbuflen) {
      if (wsctx->readbuflen > len) {
	memcpy(dst, wsctx->readbuf +  wsctx->readbufstart, len);
	result = len;
	wsctx->readbuflen -= len;
	wsctx->readbufstart += len;
      } else {
	memcpy(dst, wsctx->readbuf +  wsctx->readbufstart, wsctx->readbuflen);
	result = wsctx->readbuflen;
	wsctx->readbuflen = 0;
	wsctx->readbufstart = 0;
      }
      goto spor;
    }
    buf = wsctx->codeBufDecode;
    header = (ws_header_t *)wsctx->codeBufDecode;
    ret = ws_peek(cl, buf, B64LEN(len) + WSHLENMAX);
    if (ret < 2) {
        if (-1 == ret) {
            int olderrno = errno;
            rfbErr("%s: peek; %m\n", __func__);
            errno = olderrno;
        } else if (0 == ret) {
            result = 0;
        } else {
            errno = EAGAIN;
        }
        goto spor;
    }
    opcode = header->b0 & 0x0f;
    flength = header->b1 & 0x7f;
    if (!(header->b1 & 0x80)) {
	rfbErr("%s: got frame without mask\n", __func__, ret);
	errno = EIO;
	goto spor;
    }
    if (flength < 126) {
	fhlen = 2;
	mask = header->u.m;
    } else if (flength == 126 && 4 <= ret) {
	flength = WS_NTOH16(header->u.s16.l16);
	fhlen = 4;
	mask = header->u.s16.m16;
    } else if (flength == 127 && 10 <= ret) {
	flength = WS_NTOH64(header->u.s64.l64);
	fhlen = 10;
	mask = header->u.s64.m64;
    } else {
      rfbErr("%s: incomplete frame header\n", __func__, ret);
      errno = EIO;
      goto spor;
    }
    total = fhlen + flength + 4;
    payload = buf + fhlen + 4;  
    if (-1 == (ret = ws_read(cl, buf, total))) {
      int olderrno = errno;
      rfbErr("%s: read; %m", __func__);
      errno = olderrno;
      return ret;
    } else if (ret < total) {
      rfbLog("%s: read; got partial data\n", __func__);
    } else {
      buf[ret] = '\0';
    }
    payload32 = (uint32_t *)payload;
    for (i = 0; i < flength / 4; i++) {
	payload32[i] ^= mask.u;
    }
    for (i*=4; i < flength; i++) {
	payload[i] ^= mask.c[i % 4];
    }
    switch (opcode) {
      case WS_OPCODE_CLOSE:
	rfbLog("got closure, reason %d\n", WS_NTOH16(((uint16_t *)payload)[0]));
	errno = ECONNRESET;
	break;
      case WS_OPCODE_TEXT_FRAME:
	if (-1 == (flength = b64_pton(payload, (unsigned char *)wsctx->codeBufDecode, sizeof(wsctx->codeBufDecode)))) {
	  rfbErr("%s: Base64 decode error; %m\n", __func__);
	  break;
	}
	payload = wsctx->codeBufDecode;
      case WS_OPCODE_BINARY_FRAME:
	if (flength > len) {
	  memcpy(wsctx->readbuf, payload + len, flength - len);
	  wsctx->readbufstart = 0;
	  wsctx->readbuflen = flength - len;
	  flength = len;
	}
	memcpy(dst, payload, flength);
	result = flength;
	break;
      default:
	rfbErr("%s: unhandled opcode %d, b0: %02x, b1: %02x\n", __func__, (int)opcode, header->b0, header->b1);
    }
spor:
    return result;
}