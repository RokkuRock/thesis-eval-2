hybiReadAndDecode(ws_ctx_t *wsctx, char *dst, int len, int *sockRet, int nInBuf)
{
  int n;
  int i;
  int toReturn;  
  int toDecode;  
  int bufsize;
  int nextRead;
  unsigned char *data;
  uint32_t *data32;
  memcpy(wsctx->writePos, wsctx->carryBuf, wsctx->carrylen);
  wsctx->writePos += wsctx->carrylen;
  bufsize = wsctx->codeBufDecode + ARRAYSIZE(wsctx->codeBufDecode) - wsctx->writePos - 1;
  ws_dbg("bufsize=%d\n", bufsize);
  if (hybiRemaining(wsctx) > bufsize) {
    nextRead = bufsize;
  } else {
    nextRead = hybiRemaining(wsctx);
  }
  ws_dbg("calling read with buf=%p and len=%d (decodebuf=%p headerLen=%d)\n", wsctx->writePos, nextRead, wsctx->codeBufDecode, wsctx->header.headerLen);
  if (nextRead > 0) {
    if (-1 == (n = wsctx->ctxInfo.readFunc(wsctx->ctxInfo.ctxPtr, wsctx->writePos, nextRead))) {
      int olderrno = errno;
      rfbErr("%s: read; %s", __func__, strerror(errno));
      errno = olderrno;
      *sockRet = -1;
      return WS_HYBI_STATE_ERR;
    } else if (n == 0) {
      *sockRet = 0;
      return WS_HYBI_STATE_ERR;
    } else {
      ws_dbg("read %d bytes from socket; nRead=%d\n", n, wsctx->nReadPayload);
    }
  } else {
    n = 0;
  }
  wsctx->nReadPayload += n;
  wsctx->writePos += n;
  if (hybiRemaining(wsctx) == 0) {
    wsctx->hybiDecodeState = WS_HYBI_STATE_FRAME_COMPLETE;
  }
  toDecode = n + wsctx->carrylen + nInBuf;
  ws_dbg("toDecode=%d from n=%d carrylen=%d headerLen=%d\n", toDecode, n, wsctx->carrylen, wsctx->header.headerLen);
  if (toDecode < 0) {
    rfbErr("%s: internal error; negative number of bytes to decode: %d", __func__, toDecode);
    errno=EIO;
    *sockRet = -1;
    return WS_HYBI_STATE_ERR;
  }
  data = (unsigned char *)(wsctx->writePos - toDecode);
  data32= (uint32_t *)data;
  for (i = 0; i < (toDecode >> 2); i++) {
    data32[i] ^= wsctx->header.mask.u;
  }
  ws_dbg("mask decoding; i=%d toDecode=%d\n", i, toDecode);
  if (wsctx->hybiDecodeState == WS_HYBI_STATE_FRAME_COMPLETE) {
    for (i*=4; i < toDecode; i++) {
      data[i] ^= wsctx->header.mask.c[i % 4];
    }
    wsctx->carrylen = 0;
  } else {
    wsctx->carrylen = toDecode - (i * 4);
    if (wsctx->carrylen < 0 || wsctx->carrylen > ARRAYSIZE(wsctx->carryBuf)) {
      rfbErr("%s: internal error, invalid carry over size: carrylen=%d, toDecode=%d, i=%d", __func__, wsctx->carrylen, toDecode, i);
      *sockRet = -1;
      errno = EIO;
      return WS_HYBI_STATE_ERR;
    }
    ws_dbg("carrying over %d bytes from %p to %p\n", wsctx->carrylen, wsctx->writePos + (i * 4), wsctx->carryBuf);
    memcpy(wsctx->carryBuf, data + (i * 4), wsctx->carrylen);
    wsctx->writePos -= wsctx->carrylen;
  }
  toReturn = toDecode - wsctx->carrylen;
  switch (wsctx->header.opcode) {
    case WS_OPCODE_CLOSE:
      if (hybiWsFrameComplete(wsctx)) {
        *(wsctx->writePos) = '\0';
        ws_dbg("got close cmd %d, reason %d: %s\n", (int)(wsctx->writePos - hybiPayloadStart(wsctx)), WS_NTOH16(((uint16_t *)hybiPayloadStart(wsctx))[0]), &hybiPayloadStart(wsctx)[2]);
        errno = ECONNRESET;
        *sockRet = -1;
        return WS_HYBI_STATE_FRAME_COMPLETE;
      } else {
        ws_dbg("got close cmd; waiting for %d more bytes to arrive\n", hybiRemaining(wsctx));
        *sockRet = -1;
        errno = EAGAIN;
        return WS_HYBI_STATE_CLOSE_REASON_PENDING;
      }
      break;
    case WS_OPCODE_TEXT_FRAME:
      data[toReturn] = '\0';
      ws_dbg("Initiate Base64 decoding in %p with max size %d and '\\0' at %p\n", data, bufsize, data + toReturn);
      if (-1 == (wsctx->readlen = rfbBase64PtoN((char *)data, data, bufsize))) {
        rfbErr("%s: Base64 decode error; %s\n", __func__, strerror(errno));
      }
      wsctx->writePos = hybiPayloadStart(wsctx);
      break;
    case WS_OPCODE_BINARY_FRAME:
      wsctx->readlen = toReturn;
      wsctx->writePos = hybiPayloadStart(wsctx);
      ws_dbg("set readlen=%d writePos=%p\n", wsctx->readlen, wsctx->writePos);
      break;
    default:
      rfbErr("%s: unhandled opcode %d, b0: %02x, b1: %02x\n", __func__, (int)wsctx->header.opcode, wsctx->header.data->b0, wsctx->header.data->b1);
  }
  wsctx->readPos = data;
  return hybiReturnData(dst, len, wsctx, sockRet);
}