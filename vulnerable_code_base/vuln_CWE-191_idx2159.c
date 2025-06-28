int decomp_get_rddata(js_string *compressed, js_string *out,
                      unsigned int compressed_offset, int type, int rdlength) {
    char *desc;
    int subtype, total, len;
    desc = decomp_get_rrdesc(type);
    if(desc == 0) {  
        if(rdlength == 0) {
            return JS_SUCCESS;
            }
        if(decomp_append_bytes(compressed,out,compressed_offset,
                               rdlength) != JS_SUCCESS) {
            return JS_ERROR;
            }
        else {
            return JS_SUCCESS;
            }
        }
    else {
        subtype = *desc;
        total = 0;
        while(subtype != 0) {
            if(subtype > 0 && subtype < 64) {
                if(decomp_append_bytes(compressed,out,
                   compressed_offset,subtype) != JS_SUCCESS) {
                       return JS_ERROR;
                       }
                total += subtype;
                compressed_offset += subtype;
                }
            else if(subtype == RRSUB_DLABEL) {
                len = decomp_append_dlabel(compressed,out,
                        compressed_offset);
                if(len == JS_ERROR) {
                    return JS_ERROR;
                    }
                total += len;
                compressed_offset += len;
                }
            else if(subtype == RRSUB_TEXT) {
                len = *(compressed->string + compressed_offset);
                len += 1;  
                if(len < 0 || len > 256) {
                    return JS_ERROR;
                    }
                if(decomp_append_bytes(compressed,out,
                                       compressed_offset,len) !=
                   JS_SUCCESS) {
                    return JS_ERROR;
                    }
                total += len;
                compressed_offset += len;
                }
            else if(subtype == RRSUB_VARIABLE) {
                len = rdlength - total;
                if(len == 0) {
                    break;
                    }
                if(decomp_append_bytes(compressed,out,
                                       compressed_offset,len) != JS_SUCCESS) {
                    return JS_ERROR;
                    }
                total += len;
                compressed_offset += len;
                }
            else {  
                return JS_ERROR;
                }
            desc++;
            if(subtype != RRSUB_VARIABLE)
                subtype = *desc;
            else
                subtype = 0;  
            }
        if(rdlength != total) {
            return JS_ERROR;
            }
        }
    return JS_SUCCESS;
    }