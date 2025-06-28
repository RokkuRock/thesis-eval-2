static bool __core_anal_fcn(RCore *core, ut64 at, ut64 from, int reftype, int depth) {
	if (depth < 0) {
		return false;
	}
	int has_next = r_config_get_i (core->config, "anal.hasnext");
	RAnalHint *hint = NULL;
	int i, nexti = 0;
	ut64 *next = NULL;
	int fcnlen;
	RAnalFunction *fcn = r_anal_function_new (core->anal);
	r_warn_if_fail (fcn);
	const char *fcnpfx = r_config_get (core->config, "anal.fcnprefix");
	if (!fcnpfx) {
		fcnpfx = "fcn";
	}
	const char *cc = r_anal_cc_default (core->anal);
	if (!cc) {
		if (r_anal_cc_once (core->anal)) {
			eprintf ("Warning: set your favourite calling convention in `e anal.cc=?`\n");
		}
		cc = "reg";
	}
	fcn->cc = r_str_constpool_get (&core->anal->constpool, cc);
	r_warn_if_fail (fcn->cc);
	hint = r_anal_hint_get (core->anal, at);
	if (hint && hint->bits == 16) {
		fcn->bits = 16;
	} else {
		fcn->bits = core->anal->bits;
	}
	fcn->addr = at;
	fcn->name = get_function_name (core, at);
	if (!fcn->name) {
		fcn->name = r_str_newf ("%s.%08"PFMT64x, fcnpfx, at);
	}
	r_anal_fcn_invalidate_read_ahead_cache ();
	do {
		RFlagItem *f;
		ut64 delta = r_anal_function_linear_size (fcn);
		if (!r_io_is_valid_offset (core->io, at + delta, !core->anal->opt.noncode)) {
			goto error;
		}
		if (r_cons_is_breaked ()) {
			break;
		}
		fcnlen = r_anal_function(core->anal, fcn, at + delta, core->anal->opt.bb_max_size, reftype);
		if (core->anal->opt.searchstringrefs) {
			r_anal_set_stringrefs (core, fcn);
		}
		if (fcnlen == 0) {
			if (core->anal->verbose) {
				eprintf ("Analyzed function size is 0 at 0x%08"PFMT64x"\n", at + delta);
			}
			goto error;
		}
		if (fcnlen < 0) {
			switch (fcnlen) {
			case R_ANAL_RET_ERROR:
			case R_ANAL_RET_NEW:
			case R_ANAL_RET_DUP:
			case R_ANAL_RET_END:
				break;
			default:
				eprintf ("Oops. Negative fcnsize at 0x%08"PFMT64x" (%d)\n", at, fcnlen);
				continue;
			}
		}
		f = r_core_flag_get_by_spaces (core->flags, fcn->addr);
		set_fcn_name_from_flag (fcn, f, fcnpfx);
		if (fcnlen == R_ANAL_RET_ERROR ||
			(fcnlen == R_ANAL_RET_END && !r_anal_function_realsize (fcn))) {  
			if (core->anal->opt.followbrokenfcnsrefs) {
				r_anal_analyze_fcn_refs (core, fcn, depth);
			}
			goto error;
		} else if (fcnlen == R_ANAL_RET_END) {  
			f = r_core_flag_get_by_spaces (core->flags, fcn->addr);
			if (f && f->name && strncmp (f->name, "sect", 4)) {  
				char *new_name = strdup (f->name);
				if (is_entry_flag (f)) {
					RListIter *iter;
					RBinSymbol *sym;
					const RList *syms = r_bin_get_symbols (core->bin);
					ut64 baddr = r_config_get_i (core->config, "bin.baddr");
					r_list_foreach (syms, iter, sym) {
						if ((sym->paddr + baddr) == fcn->addr && !strcmp (sym->type, R_BIN_TYPE_FUNC_STR)) {
							free (new_name);
							new_name = r_str_newf ("sym.%s", sym->name);
							break;
						}
					}
				}
				free (fcn->name);
				fcn->name = new_name;
			} else {
				R_FREE (fcn->name);
				const char *fcnpfx = r_anal_fcntype_tostring (fcn->type);
				if (!fcnpfx || !*fcnpfx || !strcmp (fcnpfx, "fcn")) {
					fcnpfx = r_config_get (core->config, "anal.fcnprefix");
				}
				fcn->name = r_str_newf ("%s.%08"PFMT64x, fcnpfx, fcn->addr);
				autoname_imp_trampoline (core, fcn);
				r_flag_space_push (core->flags, R_FLAGS_FS_FUNCTIONS);
				r_flag_set (core->flags, fcn->name, fcn->addr, r_anal_function_linear_size (fcn));
				r_flag_space_pop (core->flags);
			}
			if (from != UT64_MAX) {
				r_anal_xrefs_set (core->anal, from, fcn->addr, reftype);
			}
			r_anal_add_function (core->anal, fcn);
			if (has_next) {
				ut64 addr = r_anal_function_max_addr (fcn);
				RIOMap *map = r_io_map_get_at (core->io, addr);
				if (!map || (map && map->perm & R_PERM_X)) {
					for (i = 0; i < nexti; i++) {
						if (next[i] == addr) {
							break;
						}
					}
					if (i == nexti) {
						ut64 at = r_anal_function_max_addr (fcn);
						while (true) {
							ut64 size;
							RAnalMetaItem *mi = r_meta_get_at (core->anal, at, R_META_TYPE_ANY, &size);
							if (!mi) {
								break;
							}
							at += size;
						}
						r_cons_clear_line (1);
						loganal (fcn->addr, at, 10000 - depth);
						next = next_append (next, &nexti, at);
					}
				}
			}
			if (!r_anal_analyze_fcn_refs (core, fcn, depth)) {
				goto error;
			}
		}
	} while (fcnlen != R_ANAL_RET_END);
	r_list_free (core->anal->leaddrs);
	core->anal->leaddrs = NULL;
	if (has_next) {
		for (i = 0; i < nexti; i++) {
			if (!next[i] || r_anal_get_fcn_in (core->anal, next[i], 0)) {
				continue;
			}
			r_core_anal_fcn (core, next[i], from, 0, depth - 1);
		}
		free (next);
	}
	if (core->anal->cur && core->anal->cur->arch && !strcmp (core->anal->cur->arch, "x86")) {
		r_anal_function_check_bp_use (fcn);
		if (fcn && !fcn->bp_frame) {
			r_anal_function_delete_vars_by_kind (fcn, R_ANAL_VAR_KIND_BPV);
		}
	}
	r_anal_hint_free (hint);
	return true;
error:
	r_list_free (core->anal->leaddrs);
	core->anal->leaddrs = NULL;
	if (fcn) {
		if (!r_anal_function_realsize (fcn) || fcn->addr == UT64_MAX) {
			r_anal_function_free (fcn);
			fcn = NULL;
		} else {
			if (!fcn->name) {
				fcn->name = r_str_newf (
					"%s.%08" PFMT64x,
					r_anal_fcntype_tostring (fcn->type),
					at);
				r_flag_space_push (core->flags, R_FLAGS_FS_FUNCTIONS);
				r_flag_set (core->flags, fcn->name, at, r_anal_function_linear_size (fcn));
				r_flag_space_pop (core->flags);
			}
			r_anal_add_function (core->anal, fcn);
		}
		if (fcn && has_next) {
			ut64 newaddr = r_anal_function_max_addr (fcn);
			RIOMap *map = r_io_map_get_at (core->io, newaddr);
			if (!map || (map && (map->perm & R_PERM_X))) {
				next = next_append (next, &nexti, newaddr);
				for (i = 0; i < nexti; i++) {
					if (!next[i]) {
						continue;
					}
					r_core_anal_fcn (core, next[i], next[i], 0, depth - 1);
				}
				free (next);
			}
		}
	}
	if (fcn && core->anal->cur && core->anal->cur->arch && !strcmp (core->anal->cur->arch, "x86")) {
		r_anal_function_check_bp_use (fcn);
		if (!fcn->bp_frame) {
			r_anal_function_delete_vars_by_kind (fcn, R_ANAL_VAR_KIND_BPV);
		}
	}
	r_anal_hint_free (hint);
	return false;
}