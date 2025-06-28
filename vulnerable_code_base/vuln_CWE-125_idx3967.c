_dwarf_internal_get_pubnames_like_data(Dwarf_Debug dbg,
    const char *secname,
    Dwarf_Small * section_data_ptr,
    Dwarf_Unsigned section_length,
    Dwarf_Global ** globals,
    Dwarf_Signed * return_count,
    Dwarf_Error * error,
    int context_DLA_code,
    int global_DLA_code,
    int length_err_num,
    int version_err_num)
{
    Dwarf_Small *pubnames_like_ptr = 0;
    Dwarf_Off pubnames_section_offset = 0;
    Dwarf_Small *section_end_ptr = section_data_ptr +section_length;
    Dwarf_Global_Context pubnames_context = 0;
    Dwarf_Bool           pubnames_context_on_list = FALSE;
    Dwarf_Unsigned version = 0;
    Dwarf_Off die_offset_in_cu = 0;
    Dwarf_Unsigned global_count = 0;
    Dwarf_Chain head_chain = 0;
    Dwarf_Chain *plast_chain = &head_chain;
    Dwarf_Global *ret_globals = 0;
    int mres = 0;
    Dwarf_Unsigned i = 0;
    if (!dbg || dbg->de_magic != DBG_IS_VALID) {
        _dwarf_error_string(NULL, error, DW_DLE_DBG_NULL,
            "DW_DLE_DBG_NULL: "
            "calling for pubnames-like data Dwarf_Debug "
            "either null or it contains"
            "a stale Dwarf_Debug pointer");
        return DW_DLV_ERROR;
    }
    if (!dbg->de_debug_info.dss_data) {
        int res = _dwarf_load_debug_info(dbg, error);
        if (res != DW_DLV_OK) {
            return res;
        }
    }
    if (section_data_ptr == NULL) {
        return DW_DLV_NO_ENTRY;
    }
    pubnames_like_ptr = section_data_ptr;
    do {
        Dwarf_Unsigned length = 0;
        int local_extension_size = 0;
        int local_length_size = 0;
        Dwarf_Small *pubnames_ptr_past_end_cu = 0;
        pubnames_context_on_list = FALSE;
        pubnames_context = (Dwarf_Global_Context)
            _dwarf_get_alloc(dbg, context_DLA_code, 1);
        if (pubnames_context == NULL) {
            dealloc_globals_chain(dbg,head_chain);
            _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
            return DW_DLV_ERROR;
        }
        if ((pubnames_like_ptr + DWARF_32BIT_SIZE +
            DWARF_HALF_SIZE + DWARF_32BIT_SIZE) >
            section_end_ptr) {
            pubnames_error_length(dbg,error,
                DWARF_32BIT_SIZE + DWARF_HALF_SIZE + DWARF_32BIT_SIZE,
                secname,
                "header-record");
            dealloc_globals_chain(dbg,head_chain);
            if (!pubnames_context_on_list) {
                dwarf_dealloc(dbg,pubnames_context,context_DLA_code);
            }
            return DW_DLV_ERROR;
        }
        mres = _dwarf_read_area_length_ck_wrapper(dbg,
            &length,&pubnames_like_ptr,&local_length_size,
            &local_extension_size,section_length,section_end_ptr,
            error);
        if (mres != DW_DLV_OK) {
            dealloc_globals_chain(dbg,head_chain);
            if (!pubnames_context_on_list) {
                dwarf_dealloc(dbg,pubnames_context,context_DLA_code);
            }
            return mres;
        }
        pubnames_context->pu_alloc_type = context_DLA_code;
        pubnames_context->pu_length_size = local_length_size;
        pubnames_context->pu_length = length;
        pubnames_context->pu_extension_size = local_extension_size;
        pubnames_context->pu_dbg = dbg;
        pubnames_context->pu_pub_offset = pubnames_section_offset;
        pubnames_ptr_past_end_cu = pubnames_like_ptr + length;
        pubnames_context->pu_pub_entries_end_ptr =
            pubnames_ptr_past_end_cu;
        if ((pubnames_like_ptr + (DWARF_HALF_SIZE) ) >
            section_end_ptr) {
            pubnames_error_length(dbg,error,
                DWARF_HALF_SIZE,
                secname,"version-number");
            dealloc_globals_chain(dbg,head_chain);
            if (!pubnames_context_on_list) {
                dwarf_dealloc(dbg,pubnames_context,context_DLA_code);
            }
            return DW_DLV_ERROR;
        }
        mres = _dwarf_read_unaligned_ck_wrapper(dbg,
            &version,pubnames_like_ptr,DWARF_HALF_SIZE,
            section_end_ptr,error);
        if (mres != DW_DLV_OK) {
            dealloc_globals_chain(dbg,head_chain);
            if (!pubnames_context_on_list) {
                dwarf_dealloc(dbg,pubnames_context,context_DLA_code);
            }
            return mres;
        }
        pubnames_context->pu_version = version;
        pubnames_like_ptr += DWARF_HALF_SIZE;
        if (version != DW_PUBNAMES_VERSION2) {
            dealloc_globals_chain(dbg,head_chain);
            if (!pubnames_context_on_list) {
                dwarf_dealloc(dbg,pubnames_context,context_DLA_code);
            }
            _dwarf_error(dbg, error, version_err_num);
            return DW_DLV_ERROR;
        }
        if ((pubnames_like_ptr + 3*pubnames_context->pu_length_size)>
            section_end_ptr) {
            pubnames_error_length(dbg,error,
                3*pubnames_context->pu_length_size,
                secname,
                "header/DIE offsets");
            dealloc_globals_chain(dbg,head_chain);
            if (!pubnames_context_on_list) {
                dwarf_dealloc(dbg,pubnames_context,context_DLA_code);
            }
            return DW_DLV_ERROR;
        }
        mres = _dwarf_read_unaligned_ck_wrapper(dbg,
            &pubnames_context->pu_offset_of_cu_header,
            pubnames_like_ptr,
            pubnames_context->pu_length_size,
            section_end_ptr,error);
        if (mres != DW_DLV_OK) {
            dealloc_globals_chain(dbg,head_chain);
            if (!pubnames_context_on_list) {
                dwarf_dealloc(dbg,pubnames_context,context_DLA_code);
            }
            return mres;
        }
        pubnames_like_ptr += pubnames_context->pu_length_size;
        FIX_UP_OFFSET_IRIX_BUG(dbg,
            pubnames_context->pu_offset_of_cu_header,
            "pubnames cu header offset");
        mres = _dwarf_read_unaligned_ck_wrapper(dbg,
            &pubnames_context->pu_info_length,
            pubnames_like_ptr,
            pubnames_context->pu_length_size,
            section_end_ptr,error);
        if (mres != DW_DLV_OK) {
            dealloc_globals_chain(dbg,head_chain);
            if (!pubnames_context_on_list) {
                dwarf_dealloc(dbg,pubnames_context,context_DLA_code);
            }
            return mres;
        }
        pubnames_like_ptr += pubnames_context->pu_length_size;
        if (pubnames_like_ptr > (section_data_ptr + section_length)) {
            dealloc_globals_chain(dbg,head_chain);
            if (!pubnames_context_on_list) {
                dwarf_dealloc(dbg,pubnames_context,context_DLA_code);
            }
            _dwarf_error(dbg, error, length_err_num);
            return DW_DLV_ERROR;
        }
        mres = _dwarf_read_unaligned_ck_wrapper(dbg,
            &die_offset_in_cu,
            pubnames_like_ptr,
            pubnames_context->pu_length_size,
            pubnames_context->pu_pub_entries_end_ptr,error);
        if (mres != DW_DLV_OK) {
            dealloc_globals_chain(dbg,head_chain);
            if (!pubnames_context_on_list) {
                dwarf_dealloc(dbg,pubnames_context,context_DLA_code);
            }
            return mres;
        }
        pubnames_like_ptr += pubnames_context->pu_length_size;
        FIX_UP_OFFSET_IRIX_BUG(dbg,
            die_offset_in_cu, "offset of die in cu");
        if (pubnames_like_ptr > (section_data_ptr + section_length)) {
            dealloc_globals_chain(dbg,head_chain);
            if (!pubnames_context_on_list) {
                dwarf_dealloc(dbg,pubnames_context,context_DLA_code);
            }
            _dwarf_error(dbg, error, length_err_num);
            return DW_DLV_ERROR;
        }
        if (!die_offset_in_cu) {
            if (dbg->de_return_empty_pubnames) {
                int res = 0;
                res = _dwarf_make_global_add_to_chain(dbg,
                    global_DLA_code,
                    pubnames_context,
                    die_offset_in_cu,
                    (unsigned char *)"",
                    &global_count,
                    &pubnames_context_on_list,
                    &plast_chain,
                    error);
                if (res != DW_DLV_OK) {
                    dealloc_globals_chain(dbg,head_chain);
                    if (!pubnames_context_on_list) {
                        dwarf_dealloc(dbg,pubnames_context,
                            context_DLA_code);
                    }
                    return res;
                }
            } else {
                dwarf_dealloc(dbg,pubnames_context,context_DLA_code);
                pubnames_context = 0;
                continue;
            }
        }
        while (die_offset_in_cu) {
            int res = 0;
            unsigned char *glname = 0;
            res = _dwarf_check_string_valid(dbg,section_data_ptr,
                pubnames_like_ptr,
                pubnames_context->pu_pub_entries_end_ptr,
                DW_DLE_STRING_OFF_END_PUBNAMES_LIKE,error);
            if (res != DW_DLV_OK) {
                dealloc_globals_chain(dbg,head_chain);
                if (!pubnames_context_on_list) {
                    dwarf_dealloc(dbg,pubnames_context,
                        context_DLA_code);
                }
                return res;
            }
            glname = (unsigned char *)pubnames_like_ptr;
            pubnames_like_ptr = pubnames_like_ptr +
                strlen((char *) pubnames_like_ptr) + 1;
            res = _dwarf_make_global_add_to_chain(dbg,
                global_DLA_code,
                pubnames_context,
                die_offset_in_cu,
                glname,
                &global_count,
                &pubnames_context_on_list,
                &plast_chain,
                error);
            if (res != DW_DLV_OK) {
                dealloc_globals_chain(dbg,head_chain);
                if (!pubnames_context_on_list) {
                    dwarf_dealloc(dbg,pubnames_context,
                        context_DLA_code);
                }
                return res;
            }
            if ((pubnames_like_ptr +
                pubnames_context->pu_length_size ) >
                section_end_ptr) {
                pubnames_error_length(dbg,error,
                    2*pubnames_context->pu_length_size,
                    secname,
                    "global record offset");
                dealloc_globals_chain(dbg,head_chain);
                if (!pubnames_context_on_list) {
                    dwarf_dealloc(dbg,pubnames_context,
                        context_DLA_code);
                }
                return DW_DLV_ERROR;
            }
            mres = _dwarf_read_unaligned_ck_wrapper(dbg,
                &die_offset_in_cu,
                pubnames_like_ptr,
                pubnames_context->pu_length_size,
                pubnames_context->pu_pub_entries_end_ptr,
                error);
            if (mres != DW_DLV_OK) {
                if (!pubnames_context_on_list) {
                    dwarf_dealloc(dbg,pubnames_context,
                        context_DLA_code);
                }
                dealloc_globals_chain(dbg,head_chain);
                return mres;
            }
            pubnames_like_ptr += pubnames_context->pu_length_size;
            FIX_UP_OFFSET_IRIX_BUG(dbg,
                die_offset_in_cu, "offset of next die in cu");
            if (pubnames_like_ptr >
                (section_data_ptr + section_length)) {
                if (!pubnames_context_on_list) {
                    dwarf_dealloc(dbg,pubnames_context,
                        context_DLA_code);
                }
                dealloc_globals_chain(dbg,head_chain);
                _dwarf_error(dbg, error, length_err_num);
                return DW_DLV_ERROR;
            }
        }
        if (pubnames_like_ptr > pubnames_ptr_past_end_cu) {
            _dwarf_error(dbg, error, length_err_num);
            if (!pubnames_context_on_list) {
                dwarf_dealloc(dbg,pubnames_context,context_DLA_code);
            }
            dealloc_globals_chain(dbg,head_chain);
            return DW_DLV_ERROR;
        }
        {
            Dwarf_Unsigned increment =
                pubnames_context->pu_length_size +
                pubnames_context->pu_length +
                pubnames_context->pu_extension_size;
            pubnames_section_offset += increment;
        }
        pubnames_like_ptr = pubnames_ptr_past_end_cu;
    } while (pubnames_like_ptr < section_end_ptr);
    ret_globals = (Dwarf_Global *)
        _dwarf_get_alloc(dbg, DW_DLA_LIST, global_count);
    if (ret_globals == NULL) {
        if (!pubnames_context_on_list) {
            dwarf_dealloc(dbg,pubnames_context,context_DLA_code);
        }
        dealloc_globals_chain(dbg,head_chain);
        _dwarf_error(dbg, error, DW_DLE_ALLOC_FAIL);
        return DW_DLV_ERROR;
    }
    {
        Dwarf_Chain curr_chain = 0;
        curr_chain = head_chain;
        for (i = 0; i < global_count; i++) {
            Dwarf_Chain prev = 0;
            *(ret_globals + i) = curr_chain->ch_item;
            prev = curr_chain;
            curr_chain = curr_chain->ch_next;
            prev->ch_item = 0;  
            dwarf_dealloc(dbg, prev, DW_DLA_CHAIN);
        }
    }
    *globals = ret_globals;
    *return_count = (Dwarf_Signed) global_count;
    return DW_DLV_OK;
}