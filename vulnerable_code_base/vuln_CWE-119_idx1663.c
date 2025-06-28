BZIP3_API s32 bz3_decode_block(struct bz3_state * state, u8 * buffer, s32 data_size, s32 orig_size) {
    u32 crc32 = read_neutral_s32(buffer);
    s32 bwt_idx = read_neutral_s32(buffer + 4);
    if (data_size > bz3_bound(state->block_size) || data_size < 0) {
        state->last_error = BZ3_ERR_MALFORMED_HEADER;
        return -1;
    }
    if (bwt_idx == -1) {
        if (data_size - 8 > 64) {
            state->last_error = BZ3_ERR_MALFORMED_HEADER;
            return -1;
        }
        memmove(buffer, buffer + 8, data_size - 8);
        if (crc32sum(1, buffer, data_size - 8) != crc32) {
            state->last_error = BZ3_ERR_CRC;
            return -1;
        }
        return data_size - 8;
    }
    s8 model = buffer[8];
    s32 lzp_size = -1, rle_size = -1, p = 0;
    if (model & 2) lzp_size = read_neutral_s32(buffer + 9 + 4 * p++);
    if (model & 4) rle_size = read_neutral_s32(buffer + 9 + 4 * p++);
    p += 2;
    data_size -= p * 4 + 1;
    if (((model & 2) && (lzp_size > bz3_bound(state->block_size) || lzp_size < 0)) ||
        ((model & 4) && (rle_size > bz3_bound(state->block_size) || rle_size < 0))) {
        state->last_error = BZ3_ERR_MALFORMED_HEADER;
        return -1;
    }
    if (orig_size > bz3_bound(state->block_size) || orig_size < 0) {
        state->last_error = BZ3_ERR_MALFORMED_HEADER;
        return -1;
    }
    u8 *b1 = buffer, *b2 = state->swap_buffer;
    begin(state->cm_state);
    state->cm_state->in_queue = b1 + p * 4 + 1;
    state->cm_state->input_ptr = 0;
    state->cm_state->input_max = data_size;
    s32 size_src;
    if (model & 2)
        size_src = lzp_size;
    else if (model & 4)
        size_src = rle_size;
    else
        size_src = orig_size;
    decode_bytes(state->cm_state, b2, size_src);
    swap(b1, b2);
    if (bwt_idx >= size_src) {
        state->last_error = BZ3_ERR_MALFORMED_HEADER;
        return -1;
    }
    if (libsais_unbwt(b1, b2, state->sais_array, size_src, NULL, bwt_idx) < 0) {
        state->last_error = BZ3_ERR_BWT;
        return -1;
    }
    swap(b1, b2);
    if (model & 2) {
        size_src = lzp_decompress(b1, b2, lzp_size, bz3_bound(state->block_size), state->lzp_lut);
        if (size_src == -1) {
            state->last_error = BZ3_ERR_CRC;
            return -1;
        }
        swap(b1, b2);
    }
    if (model & 4) {
        int err = mrled(b1, b2, orig_size, size_src);
        if(err) {
            state->last_error = BZ3_ERR_CRC;
            return -1;
        }
        size_src = orig_size;
        swap(b1, b2);
    }
    state->last_error = BZ3_OK;
    if (size_src > bz3_bound(state->block_size) || size_src < 0) {
        state->last_error = BZ3_ERR_MALFORMED_HEADER;
        return -1;
    }
    if (b1 != buffer) memcpy(buffer, b1, size_src);
    if (crc32 != crc32sum(1, buffer, size_src)) {
        state->last_error = BZ3_ERR_CRC;
        return -1;
    }
    return size_src;
}