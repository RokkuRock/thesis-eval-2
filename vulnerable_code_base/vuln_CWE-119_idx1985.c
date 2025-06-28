static OPJ_BOOL opj_tcd_code_block_enc_allocate_data(opj_tcd_cblk_enc_t *
        p_code_block)
{
    OPJ_UINT32 l_data_size;
    l_data_size = (OPJ_UINT32)((p_code_block->x1 - p_code_block->x0) *
                               (p_code_block->y1 - p_code_block->y0) * (OPJ_INT32)sizeof(OPJ_UINT32));
    if (l_data_size > p_code_block->data_size) {
        if (p_code_block->data) {
            opj_free(p_code_block->data - 1);
        }
        p_code_block->data = (OPJ_BYTE*) opj_malloc(l_data_size + 1);
        if (! p_code_block->data) {
            p_code_block->data_size = 0U;
            return OPJ_FALSE;
        }
        p_code_block->data_size = l_data_size;
        p_code_block->data[0] = 0;
        p_code_block->data += 1;  
    }
    return OPJ_TRUE;
}