int ZEXPORT deflatePrime (strm, bits, value)
    z_streamp strm;
    int bits;
    int value;
{
    deflate_state *s;
    int put;
    if (deflateStateCheck(strm)) return Z_STREAM_ERROR;
    s = strm->state;
    if ((Bytef *)(s->d_buf) < s->pending_out + ((Buf_size + 7) >> 3))
        return Z_BUF_ERROR;
    do {
        put = Buf_size - s->bi_valid;
        if (put > bits)
            put = bits;
        s->bi_buf |= (ush)((value & ((1 << put) - 1)) << s->bi_valid);
        s->bi_valid += put;
        _tr_flush_bits(s);
        value >>= put;
        bits -= put;
    } while (bits);
    return Z_OK;
}