did_set_string_option (
    int opt_idx,                             
    char_u **varp,                      
    int new_value_alloced,                   
    char_u *oldval,                     
    char_u *errbuf,                     
    int opt_flags                           
)
{
  char_u      *errmsg = NULL;
  char_u      *s, *p;
  int did_chartab = FALSE;
  char_u      **gvarp;
  bool free_oldval = (options[opt_idx].flags & P_ALLOCED);
  gvarp = (char_u **)get_varp_scope(&(options[opt_idx]), OPT_GLOBAL);
  if ((secure || sandbox != 0)
      && (options[opt_idx].flags & P_SECURE)) {
    errmsg = e_secure;
  }
  else if ((options[opt_idx].flags & P_NFNAME)
           && vim_strpbrk(*varp, (char_u *)"/\\*?[|<>") != NULL) {
    errmsg = e_invarg;
  }
  else if (gvarp == &p_bkc) {
    char_u       *bkc   = p_bkc;
    unsigned int *flags = &bkc_flags;
    if (opt_flags & OPT_LOCAL) {
      bkc   = curbuf->b_p_bkc;
      flags = &curbuf->b_bkc_flags;
    }
    if ((opt_flags & OPT_LOCAL) && *bkc == NUL) {
      *flags = 0;
    } else {
      if (opt_strings_flags(bkc, p_bkc_values, flags, true) != OK) {
        errmsg = e_invarg;
      }
      if (((*flags & BKC_AUTO) != 0)
          + ((*flags & BKC_YES) != 0)
          + ((*flags & BKC_NO) != 0) != 1) {
        (void)opt_strings_flags(oldval, p_bkc_values, flags, true);
        errmsg = e_invarg;
      }
    }
  }
  else if (varp == &p_bex || varp == &p_pm) {
    if (STRCMP(*p_bex == '.' ? p_bex + 1 : p_bex,
            *p_pm == '.' ? p_pm + 1 : p_pm) == 0)
      errmsg = (char_u *)N_("E589: 'backupext' and 'patchmode' are equal");
  }
  else if (varp == &curwin->w_p_briopt) {
    if (briopt_check(curwin) == FAIL)
      errmsg = e_invarg;
  } else if (varp == &p_isi
             || varp == &(curbuf->b_p_isk)
             || varp == &p_isp
             || varp == &p_isf) {
    if (init_chartab() == FAIL) {
      did_chartab = TRUE;            
      errmsg = e_invarg;             
    }
  }
  else if (varp == &p_hf) {
    if (didset_vim) {
      vim_setenv("VIM", "");
      didset_vim = FALSE;
    }
    if (didset_vimruntime) {
      vim_setenv("VIMRUNTIME", "");
      didset_vimruntime = FALSE;
    }
  }
  else if (varp == &curwin->w_p_cc)
    errmsg = check_colorcolumn(curwin);
  else if (varp == &p_hlg) {
    for (s = p_hlg; *s != NUL; s += 3) {
      if (s[1] == NUL || ((s[2] != ',' || s[3] == NUL) && s[2] != NUL)) {
        errmsg = e_invarg;
        break;
      }
      if (s[2] == NUL)
        break;
    }
  }
  else if (varp == &p_hl) {
    if (highlight_changed() == FAIL)
      errmsg = e_invarg;         
  }
  else if (gvarp == &p_nf) {
    if (check_opt_strings(*varp, p_nf_values, TRUE) != OK)
      errmsg = e_invarg;
  } else if (varp == &p_ssop) {   
    if (opt_strings_flags(p_ssop, p_ssop_values, &ssop_flags, true) != OK)
      errmsg = e_invarg;
    if ((ssop_flags & SSOP_CURDIR) && (ssop_flags & SSOP_SESDIR)) {
      (void)opt_strings_flags(oldval, p_ssop_values, &ssop_flags, true);
      errmsg = e_invarg;
    }
  } else if (varp == &p_vop) {   
    if (opt_strings_flags(p_vop, p_ssop_values, &vop_flags, true) != OK)
      errmsg = e_invarg;
  }
  else if (varp == &p_sbo) {
    if (check_opt_strings(p_sbo, p_scbopt_values, TRUE) != OK)
      errmsg = e_invarg;
  } else if (varp == &p_ambw || (int *)varp == &p_emoji) {
    if (check_opt_strings(p_ambw, p_ambw_values, false) != OK) {
      errmsg = e_invarg;
    } else if (set_chars_option(&p_lcs) != NULL) {
      errmsg = (char_u *)_("E834: Conflicts with value of 'listchars'");
    } else if (set_chars_option(&p_fcs) != NULL) {
      errmsg = (char_u *)_("E835: Conflicts with value of 'fillchars'");
    }
  }
  else if (varp == &p_bg) {
    if (check_opt_strings(p_bg, p_bg_values, FALSE) == OK) {
      int dark = (*p_bg == 'd');
      init_highlight(FALSE, FALSE);
      if (dark != (*p_bg == 'd')
          && get_var_value((char_u *)"g:colors_name") != NULL) {
        do_unlet((char_u *)"g:colors_name", TRUE);
        free_string_option(p_bg);
        p_bg = vim_strsave((char_u *)(dark ? "dark" : "light"));
        check_string_option(&p_bg);
        init_highlight(FALSE, FALSE);
      }
    } else
      errmsg = e_invarg;
  }
  else if (varp == &p_wim) {
    if (check_opt_wim() == FAIL)
      errmsg = e_invarg;
  }
  else if (varp == &p_wop) {
    if (check_opt_strings(p_wop, p_wop_values, TRUE) != OK)
      errmsg = e_invarg;
  }
  else if (varp == &p_wak) {
    if (*p_wak == NUL
        || check_opt_strings(p_wak, p_wak_values, FALSE) != OK)
      errmsg = e_invarg;
  }
  else if (varp == &p_ei) {
    if (check_ei() == FAIL)
      errmsg = e_invarg;
  } else if (varp == &p_enc || gvarp == &p_fenc) {
    if (gvarp == &p_fenc) {
      if (!MODIFIABLE(curbuf) && opt_flags != OPT_GLOBAL) {
        errmsg = e_modifiable;
      } else if (vim_strchr(*varp, ',') != NULL) {
        errmsg = e_invarg;
      } else {
        redraw_titles();
        ml_setflags(curbuf);
      }
    }
    if (errmsg == NULL) {
      p = enc_canonize(*varp);
      xfree(*varp);
      *varp = p;
      if (varp == &p_enc) {
        if (STRCMP(p_enc, "utf-8") != 0) {
          errmsg = e_invarg;
        }
      }
    }
  } else if (varp == &p_penc) {
    p = enc_canonize(p_penc);
    xfree(p_penc);
    p_penc = p;
  } else if (varp == &curbuf->b_p_keymap) {
    errmsg = keymap_init();
    if (errmsg == NULL) {
      if (*curbuf->b_p_keymap != NUL) {
        curbuf->b_p_iminsert = B_IMODE_LMAP;
        if (curbuf->b_p_imsearch != B_IMODE_USE_INSERT)
          curbuf->b_p_imsearch = B_IMODE_LMAP;
      } else {
        if (curbuf->b_p_iminsert == B_IMODE_LMAP)
          curbuf->b_p_iminsert = B_IMODE_NONE;
        if (curbuf->b_p_imsearch == B_IMODE_LMAP)
          curbuf->b_p_imsearch = B_IMODE_USE_INSERT;
      }
      if ((opt_flags & OPT_LOCAL) == 0) {
        set_iminsert_global();
        set_imsearch_global();
      }
      status_redraw_curbuf();
    }
  }
  else if (gvarp == &p_ff) {
    if (!MODIFIABLE(curbuf) && !(opt_flags & OPT_GLOBAL))
      errmsg = e_modifiable;
    else if (check_opt_strings(*varp, p_ff_values, FALSE) != OK)
      errmsg = e_invarg;
    else {
      redraw_titles();
      ml_setflags(curbuf);
      if (get_fileformat(curbuf) == EOL_MAC || *oldval == 'm')
        redraw_curbuf_later(NOT_VALID);
    }
  }
  else if (varp == &p_ffs) {
    if (check_opt_strings(p_ffs, p_ff_values, TRUE) != OK) {
      errmsg = e_invarg;
    }
  }
  else if (gvarp == &p_mps) {
    if (has_mbyte) {
      for (p = *varp; *p != NUL; ++p) {
        int x2 = -1;
        int x3 = -1;
        if (*p != NUL)
          p += mb_ptr2len(p);
        if (*p != NUL)
          x2 = *p++;
        if (*p != NUL) {
          x3 = mb_ptr2char(p);
          p += mb_ptr2len(p);
        }
        if (x2 != ':' || x3 == -1 || (*p != NUL && *p != ',')) {
          errmsg = e_invarg;
          break;
        }
        if (*p == NUL)
          break;
      }
    } else {
      for (p = *varp; *p != NUL; p += 4) {
        if (p[1] != ':' || p[2] == NUL || (p[3] != NUL && p[3] != ',')) {
          errmsg = e_invarg;
          break;
        }
        if (p[3] == NUL)
          break;
      }
    }
  }
  else if (gvarp == &p_com) {
    for (s = *varp; *s; ) {
      while (*s && *s != ':') {
        if (vim_strchr((char_u *)COM_ALL, *s) == NULL
            && !ascii_isdigit(*s) && *s != '-') {
          errmsg = illegal_char(errbuf, *s);
          break;
        }
        ++s;
      }
      if (*s++ == NUL)
        errmsg = (char_u *)N_("E524: Missing colon");
      else if (*s == ',' || *s == NUL)
        errmsg = (char_u *)N_("E525: Zero length string");
      if (errmsg != NULL)
        break;
      while (*s && *s != ',') {
        if (*s == '\\' && s[1] != NUL)
          ++s;
        ++s;
      }
      s = skip_to_option_part(s);
    }
  }
  else if (varp == &p_lcs) {
    errmsg = set_chars_option(varp);
  }
  else if (varp == &p_fcs) {
    errmsg = set_chars_option(varp);
  }
  else if (varp == &p_cedit) {
    errmsg = check_cedit();
  }
  else if (varp == &p_vfile) {
    verbose_stop();
    if (*p_vfile != NUL && verbose_open() == FAIL)
      errmsg = e_invarg;
  } else if (varp == &p_shada) {
    opt_idx = ((options[opt_idx].fullname[0] == 'v')
               ? (shada_idx == -1
                  ? ((shada_idx = findoption((char_u *) "shada")))
                  : shada_idx)
               : opt_idx);
    for (s = p_shada; *s; ) {
      if (vim_strchr((char_u *)"!\"%'/:<@cfhnrs", *s) == NULL) {
        errmsg = illegal_char(errbuf, *s);
        break;
      }
      if (*s == 'n') {           
        break;
      } else if (*s == 'r') {  
        while (*++s && *s != ',')
          ;
      } else if (*s == '%') {
        while (ascii_isdigit(*++s))
          ;
      } else if (*s == '!' || *s == 'h' || *s == 'c')
        ++s;                     
      else {                     
        while (ascii_isdigit(*++s))
          ;
        if (!ascii_isdigit(*(s - 1))) {
          if (errbuf != NULL) {
            sprintf((char *)errbuf,
                _("E526: Missing number after <%s>"),
                transchar_byte(*(s - 1)));
            errmsg = errbuf;
          } else
            errmsg = (char_u *)"";
          break;
        }
      }
      if (*s == ',')
        ++s;
      else if (*s) {
        if (errbuf != NULL)
          errmsg = (char_u *)N_("E527: Missing comma");
        else
          errmsg = (char_u *)"";
        break;
      }
    }
    if (*p_shada && errmsg == NULL && get_shada_parameter('\'') < 0)
      errmsg = (char_u *)N_("E528: Must specify a ' value");
  }
  else if (varp == &p_sbr) {
    for (s = p_sbr; *s; ) {
      if (ptr2cells(s) != 1)
        errmsg = (char_u *)N_("E595: contains unprintable or wide character");
      mb_ptr_adv(s);
    }
  }
  else if (varp == &p_guicursor)
    errmsg = parse_shape_opt(SHAPE_CURSOR);
  else if (varp == &p_popt)
    errmsg = parse_printoptions();
  else if (varp == &p_pmfn)
    errmsg = parse_printmbfont();
  else if (varp == &p_langmap)
    langmap_set();
  else if (varp == &p_breakat)
    fill_breakat_flags();
  else if (varp == &p_titlestring || varp == &p_iconstring) {
    int flagval = (varp == &p_titlestring) ? STL_IN_TITLE : STL_IN_ICON;
    if (vim_strchr(*varp, '%') && check_stl_option(*varp) == NULL)
      stl_syntax |= flagval;
    else
      stl_syntax &= ~flagval;
    did_set_title(varp == &p_iconstring);
  }
  else if (varp == &p_sel) {
    if (*p_sel == NUL
        || check_opt_strings(p_sel, p_sel_values, FALSE) != OK)
      errmsg = e_invarg;
  }
  else if (varp == &p_slm) {
    if (check_opt_strings(p_slm, p_slm_values, TRUE) != OK)
      errmsg = e_invarg;
  }
  else if (varp == &p_km) {
    if (check_opt_strings(p_km, p_km_values, TRUE) != OK)
      errmsg = e_invarg;
    else {
      km_stopsel = (vim_strchr(p_km, 'o') != NULL);
      km_startsel = (vim_strchr(p_km, 'a') != NULL);
    }
  }
  else if (varp == &p_mousem) {
    if (check_opt_strings(p_mousem, p_mousem_values, FALSE) != OK)
      errmsg = e_invarg;
  } else if (varp == &p_swb) {   
    if (opt_strings_flags(p_swb, p_swb_values, &swb_flags, true) != OK)
      errmsg = e_invarg;
  }
  else if (varp == &p_debug) {
    if (check_opt_strings(p_debug, p_debug_values, TRUE) != OK)
      errmsg = e_invarg;
  } else if (varp == &p_dy) {   
    if (opt_strings_flags(p_dy, p_dy_values, &dy_flags, true) != OK)
      errmsg = e_invarg;
    else
      (void)init_chartab();
  }
  else if (varp == &p_ead) {
    if (check_opt_strings(p_ead, p_ead_values, FALSE) != OK)
      errmsg = e_invarg;
  } else if (varp == &p_cb) {   
    if (opt_strings_flags(p_cb, p_cb_values, &cb_flags, true) != OK) {
      errmsg = e_invarg;
    }
  } else if (varp == &(curwin->w_s->b_p_spl)   
             || varp == &(curwin->w_s->b_p_spf)) {
    errmsg = did_set_spell_option(varp == &(curwin->w_s->b_p_spf));
  }
  else if (varp == &(curwin->w_s->b_p_spc)) {
    errmsg = compile_cap_prog(curwin->w_s);
  }
  else if (varp == &p_sps) {
    if (spell_check_sps() != OK)
      errmsg = e_invarg;
  }
  else if (varp == &p_msm) {
    if (spell_check_msm() != OK)
      errmsg = e_invarg;
  }
  else if (gvarp == &p_bh) {
    if (check_opt_strings(curbuf->b_p_bh, p_bufhidden_values, FALSE) != OK)
      errmsg = e_invarg;
  }
  else if (gvarp == &p_bt) {
    if ((curbuf->terminal && curbuf->b_p_bt[0] != 't')
        || (!curbuf->terminal && curbuf->b_p_bt[0] == 't')
        || check_opt_strings(curbuf->b_p_bt, p_buftype_values, FALSE) != OK) {
      errmsg = e_invarg;
    } else {
      if (curwin->w_status_height) {
        curwin->w_redr_status = TRUE;
        redraw_later(VALID);
      }
      curbuf->b_help = (curbuf->b_p_bt[0] == 'h');
      redraw_titles();
    }
  }
  else if (gvarp == &p_stl || varp == &p_ruf) {
    int wid;
    if (varp == &p_ruf)          
      ru_wid = 0;
    s = *varp;
    if (varp == &p_ruf && *s == '%') {
      if (*++s == '-')           
        s++;
      wid = getdigits_int(&s);
      if (wid && *s == '(' && (errmsg = check_stl_option(p_ruf)) == NULL)
        ru_wid = wid;
      else
        errmsg = check_stl_option(p_ruf);
    }
    else if (varp == &p_ruf || s[0] != '%' || s[1] != '!')
      errmsg = check_stl_option(s);
    if (varp == &p_ruf && errmsg == NULL)
      comp_col();
  }
  else if (gvarp == &p_cpt) {
    for (s = *varp; *s; ) {
      while (*s == ',' || *s == ' ')
        s++;
      if (!*s)
        break;
      if (vim_strchr((char_u *)".wbuksid]tU", *s) == NULL) {
        errmsg = illegal_char(errbuf, *s);
        break;
      }
      if (*++s != NUL && *s != ',' && *s != ' ') {
        if (s[-1] == 'k' || s[-1] == 's') {
          while (*s && *s != ',' && *s != ' ') {
            if (*s == '\\')
              ++s;
            ++s;
          }
        } else {
          if (errbuf != NULL) {
            sprintf((char *)errbuf,
                _("E535: Illegal character after <%c>"),
                *--s);
            errmsg = errbuf;
          } else
            errmsg = (char_u *)"";
          break;
        }
      }
    }
  }
  else if (varp == &p_cot) {
    if (check_opt_strings(p_cot, p_cot_values, true) != OK) {
      errmsg = e_invarg;
    } else {
      completeopt_was_set();
    }
  }
  else if (varp == &p_pt) {
    if (*p_pt) {
      (void)replace_termcodes(p_pt, STRLEN(p_pt), &p, true, true, false,
                              CPO_TO_CPO_FLAGS);
      if (p != NULL) {
        if (new_value_alloced)
          free_string_option(p_pt);
        p_pt = p;
        new_value_alloced = TRUE;
      }
    }
  }
  else if (varp == &p_bs) {
    if (ascii_isdigit(*p_bs)) {
      if (*p_bs >'2' || p_bs[1] != NUL)
        errmsg = e_invarg;
    } else if (check_opt_strings(p_bs, p_bs_values, TRUE) != OK)
      errmsg = e_invarg;
  } else if (varp == &p_bo) {
    if (opt_strings_flags(p_bo, p_bo_values, &bo_flags, true) != OK) {
      errmsg = e_invarg;
    }
  } else if (gvarp == &p_tc) {   
    unsigned int *flags;
    if (opt_flags & OPT_LOCAL) {
      p = curbuf->b_p_tc;
      flags = &curbuf->b_tc_flags;
    } else {
      p = p_tc;
      flags = &tc_flags;
    }
    if ((opt_flags & OPT_LOCAL) && *p == NUL) {
      *flags = 0;
    } else if (*p == NUL
               || opt_strings_flags(p, p_tc_values, flags, false) != OK) {
      errmsg = e_invarg;
    }
  } else if (varp == &p_cmp) {   
    if (opt_strings_flags(p_cmp, p_cmp_values, &cmp_flags, true) != OK)
      errmsg = e_invarg;
  }
  else if (varp == &p_dip) {
    if (diffopt_changed() == FAIL)
      errmsg = e_invarg;
  }
  else if (gvarp == &curwin->w_allbuf_opt.wo_fdm) {
    if (check_opt_strings(*varp, p_fdm_values, FALSE) != OK
        || *curwin->w_p_fdm == NUL)
      errmsg = e_invarg;
    else {
      foldUpdateAll(curwin);
      if (foldmethodIsDiff(curwin))
        newFoldLevel();
    }
  }
  else if (varp == &curwin->w_p_fde) {
    if (foldmethodIsExpr(curwin))
      foldUpdateAll(curwin);
  }
  else if (gvarp == &curwin->w_allbuf_opt.wo_fmr) {
    p = vim_strchr(*varp, ',');
    if (p == NULL)
      errmsg = (char_u *)N_("E536: comma required");
    else if (p == *varp || p[1] == NUL)
      errmsg = e_invarg;
    else if (foldmethodIsMarker(curwin))
      foldUpdateAll(curwin);
  }
  else if (gvarp == &p_cms) {
    if (**varp != NUL && strstr((char *)*varp, "%s") == NULL)
      errmsg = (char_u *)N_(
                "E537: 'commentstring' must be empty or contain %s");
  } else if (varp == &p_fdo) {   
    if (opt_strings_flags(p_fdo, p_fdo_values, &fdo_flags, true) != OK)
      errmsg = e_invarg;
  }
  else if (varp == &p_fcl) {
    if (check_opt_strings(p_fcl, p_fcl_values, TRUE) != OK)
      errmsg = e_invarg;
  }
  else if (gvarp == &curwin->w_allbuf_opt.wo_fdi) {
    if (foldmethodIsIndent(curwin))
      foldUpdateAll(curwin);
  } else if (varp == &p_ve) {   
    if (opt_strings_flags(p_ve, p_ve_values, &ve_flags, true) != OK)
      errmsg = e_invarg;
    else if (STRCMP(p_ve, oldval) != 0) {
      validate_virtcol();
      coladvance(curwin->w_virtcol);
    }
  } else if (varp == &p_csqf) {
    if (p_csqf != NULL) {
      p = p_csqf;
      while (*p != NUL) {
        if (vim_strchr((char_u *)CSQF_CMDS, *p) == NULL
            || p[1] == NUL
            || vim_strchr((char_u *)CSQF_FLAGS, p[1]) == NULL
            || (p[2] != NUL && p[2] != ',')) {
          errmsg = e_invarg;
          break;
        } else if (p[2] == NUL)
          break;
        else
          p += 3;
      }
    }
  }
  else if (gvarp == &p_cino) {
    parse_cino(curbuf);
  } else if (varp == &p_icm) {
      if (check_opt_strings(p_icm, p_icm_values, false) != OK) {
        errmsg = e_invarg;
      }
  } else {
    p = NULL;
    if (varp == &p_ww)
      p = (char_u *)WW_ALL;
    if (varp == &p_shm)
      p = (char_u *)SHM_ALL;
    else if (varp == &(p_cpo))
      p = (char_u *)CPO_VI;
    else if (varp == &(curbuf->b_p_fo))
      p = (char_u *)FO_ALL;
    else if (varp == &curwin->w_p_cocu)
      p = (char_u *)COCU_ALL;
    else if (varp == &p_mouse) {
      p = (char_u *)MOUSE_ALL;
    }
    if (p != NULL) {
      for (s = *varp; *s; ++s)
        if (vim_strchr(p, *s) == NULL) {
          errmsg = illegal_char(errbuf, *s);
          break;
        }
    }
  }
  if (errmsg != NULL) {
    if (new_value_alloced)
      free_string_option(*varp);
    *varp = oldval;
    if (did_chartab)
      (void)init_chartab();
    if (varp == &p_hl)
      (void)highlight_changed();
  } else {
    set_option_scriptID_idx(opt_idx, opt_flags, current_SID);
    if (free_oldval)
      free_string_option(oldval);
    if (new_value_alloced)
      options[opt_idx].flags |= P_ALLOCED;
    else
      options[opt_idx].flags &= ~P_ALLOCED;
    if ((opt_flags & (OPT_LOCAL | OPT_GLOBAL)) == 0
        && ((int)options[opt_idx].indir & PV_BOTH)) {
      p = get_varp_scope(&(options[opt_idx]), OPT_LOCAL);
      free_string_option(*(char_u **)p);
      *(char_u **)p = empty_option;
    }
    else if (!(opt_flags & OPT_LOCAL) && opt_flags != OPT_GLOBAL)
      set_string_option_global(opt_idx, varp);
    if (varp == &(curbuf->b_p_syn)) {
      apply_autocmds(EVENT_SYNTAX, curbuf->b_p_syn,
          curbuf->b_fname, TRUE, curbuf);
    } else if (varp == &(curbuf->b_p_ft)) {
      did_filetype = TRUE;
      apply_autocmds(EVENT_FILETYPE, curbuf->b_p_ft,
          curbuf->b_fname, TRUE, curbuf);
    }
    if (varp == &(curwin->w_s->b_p_spl)) {
      char_u fname[200];
      char_u      *q = curwin->w_s->b_p_spl;
      if (STRNCMP(q, "cjk,", 4) == 0)
        q += 4;
      for (p = q; *p != NUL; ++p)
        if (vim_strchr((char_u *)"_.,", *p) != NULL)
          break;
      vim_snprintf((char *)fname, sizeof(fname), "spell/%.*s.vim",
                   (int)(p - q), q);
      source_runtime(fname, DIP_ALL);
    }
  }
  if (varp == &p_mouse) {
    if (*p_mouse == NUL) {
      ui_mouse_off();
    } else {
      setmouse();   
    }
  }
  if (curwin->w_curswant != MAXCOL
      && (options[opt_idx].flags & (P_CURSWANT | P_RALL)) != 0)
    curwin->w_set_curswant = TRUE;
  check_redraw(options[opt_idx].flags);
  return errmsg;
}