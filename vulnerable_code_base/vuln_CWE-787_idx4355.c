BZIP3_API struct bz3_state * bz3_new(s32 block_size) {
    if (block_size < KiB(65) || block_size > MiB(511)) {
        return NULL;
    }
    struct bz3_state * bz3_state = malloc(sizeof(struct bz3_state));
    if (!bz3_state) {
        return NULL;
    }
    bz3_state->cm_state = malloc(sizeof(state));
    bz3_state->swap_buffer = malloc(bz3_bound(block_size));
    bz3_state->sais_array = malloc((block_size + 128) * sizeof(s32));
    memset(bz3_state->sais_array, 0, sizeof(s32) * (block_size + 128));
    bz3_state->lzp_lut = calloc(1 << LZP_DICTIONARY, sizeof(s32));
    if (!bz3_state->cm_state || !bz3_state->swap_buffer || !bz3_state->sais_array || !bz3_state->lzp_lut) {
        if (bz3_state->cm_state) free(bz3_state->cm_state);
        if (bz3_state->swap_buffer) free(bz3_state->swap_buffer);
        if (bz3_state->sais_array) free(bz3_state->sais_array);
        if (bz3_state->lzp_lut) free(bz3_state->lzp_lut);
        free(bz3_state);
        return NULL;
    }
    bz3_state->block_size = block_size;
    bz3_state->last_error = BZ3_OK;
    return bz3_state;
}