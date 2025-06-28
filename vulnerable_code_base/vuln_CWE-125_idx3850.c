S_grok_bslash_N(pTHX_ RExC_state_t *pRExC_state,
                regnode ** node_p,
                UV * code_point_p,
                int * cp_count,
                I32 * flagp,
                const bool strict,
                const U32 depth
    )
{
    char * endbrace;     
    char *endchar;	 
    char* p = RExC_parse;  
    GET_RE_DEBUG_FLAGS_DECL;
    PERL_ARGS_ASSERT_GROK_BSLASH_N;
    GET_RE_DEBUG_FLAGS;
    assert(cBOOL(node_p) ^ cBOOL(code_point_p));   
    assert(! (node_p && cp_count));                
    if (cp_count) {      
        *cp_count = 1;
    }
    skip_to_be_ignored_text(pRExC_state, &p,
                            FALSE   );
    if (*p != '{' || regcurly(p)) {
	RExC_parse = p;
        if (cp_count) {
            *cp_count = -1;
        }
	if (! node_p) {
            return FALSE;
        }
	*node_p = reg_node(pRExC_state, REG_ANY);
	*flagp |= HASWIDTH|SIMPLE;
	MARK_NAUGHTY(1);
        Set_Node_Length(*node_p, 1);  
	return TRUE;
    }
    if (*RExC_parse != '{') {
	vFAIL("Missing braces on \\N{}");
    }
    RExC_parse++;	 
    endbrace = strchr(RExC_parse, '}');
    if (! endbrace) {  
        vFAIL2("Missing right brace on \\%c{}", 'N');
    }
    else if (!(   endbrace == RExC_parse	 
               || memBEGINs(RExC_parse,    
                                  (STRLEN) (RExC_end - RExC_parse),
                                 "U+")))
    {
	RExC_parse = endbrace;	 
	vFAIL("\\N{NAME} must be resolved by the lexer");
    }
    REQUIRE_UNI_RULES(flagp, FALSE);  
    if (endbrace == RExC_parse) {    
        if (strict) {
            RExC_parse++;    
            vFAIL("Zero length \\N{}");
        }
        if (cp_count) {
            *cp_count = 0;
        }
        nextchar(pRExC_state);
	if (! node_p) {
            return FALSE;
        }
        *node_p = reg_node(pRExC_state,NOTHING);
        return TRUE;
    }
    RExC_parse += 2;	 
    endchar = RExC_parse + strcspn(RExC_parse, ".}");
    if (endchar >= endbrace) {
	STRLEN length_of_hex;
	I32 grok_hex_flags;
        if (! code_point_p) {
            RExC_parse = p;
            return FALSE;
        }
	length_of_hex = (STRLEN)(endchar - RExC_parse);
	grok_hex_flags = PERL_SCAN_ALLOW_UNDERSCORES
                       | PERL_SCAN_DISALLOW_PREFIX
                       | ((SIZE_ONLY)
                          ? PERL_SCAN_SILENT_ILLDIGIT
                          : 0);
	*code_point_p = UNI_TO_NATIVE(grok_hex(RExC_parse,
                                               &length_of_hex,
                                               &grok_hex_flags,
                                               NULL));
        if (length_of_hex == 0
            || length_of_hex != (STRLEN)(endchar - RExC_parse) )
        {
            RExC_parse += length_of_hex;	 
            RExC_parse += (RExC_orig_utf8)	 
                            ? UTF8SKIP(RExC_parse)
                            : 1;
            if (RExC_parse >= endchar) {
                RExC_parse = endchar;
            }
            vFAIL("Invalid hexadecimal number in \\N{U+...}");
        }
        RExC_parse = endbrace + 1;
        return TRUE;
    }
    else {   
	SV * substitute_parse;
	STRLEN len;
	char *orig_end = RExC_end;
	char *save_start = RExC_start;
        I32 flags;
        if (cp_count) {
            *cp_count = 0;
            while (RExC_parse < endbrace) {
                RExC_parse = endchar + 1;
                endchar = RExC_parse + strcspn(RExC_parse, ".}");
                (*cp_count)++;
            }
        }
        if (! node_p) {
            if (! cp_count) {
                RExC_parse = p;
            }
            return FALSE;
        }
	substitute_parse = newSVpvs("?:");
	while (RExC_parse < endbrace) {
	    sv_catpv(substitute_parse, "\\x{");
	    sv_catpvn(substitute_parse, RExC_parse, endchar - RExC_parse);
	    sv_catpv(substitute_parse, "}");
	    RExC_parse = endchar + 1;
	    endchar = RExC_parse + strcspn(RExC_parse, ".}");
	}
        sv_catpv(substitute_parse, ")");
        len = SvCUR(substitute_parse);
	if (len < (STRLEN) 8) {
            RExC_parse = endbrace;
	    vFAIL("Invalid hexadecimal number in \\N{U+...}");
	}
        RExC_parse = RExC_start = RExC_adjusted_start
                                              = SvPV_nolen(substitute_parse);
	RExC_end = RExC_parse + len;
#ifdef EBCDIC
        RExC_recode_x_to_native = 1;
#endif
        *node_p = reg(pRExC_state, 1, &flags, depth+1);
	RExC_start = RExC_adjusted_start = save_start;
	RExC_parse = endbrace;
	RExC_end = orig_end;
#ifdef EBCDIC
        RExC_recode_x_to_native = 0;
#endif
        SvREFCNT_dec_NN(substitute_parse);
        if (! *node_p) {
            if (flags & (RESTART_PASS1|NEED_UTF8)) {
                *flagp = flags & (RESTART_PASS1|NEED_UTF8);
                return FALSE;
            }
            FAIL2("panic: reg returned NULL to grok_bslash_N, flags=%#" UVxf,
                (UV) flags);
        }
        *flagp |= flags&(HASWIDTH|SPSTART|SIMPLE|POSTPONED);
        nextchar(pRExC_state);
        return TRUE;
    }
}