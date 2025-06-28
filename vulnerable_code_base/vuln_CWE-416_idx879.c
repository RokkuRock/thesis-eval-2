R_API void r_core_anal_type_match(RCore *core, RAnalFunction *fcn) {
	RAnalBlock *bb;
	RListIter *it;
	RAnalOp aop = {0};
	bool resolved = false;
	r_return_if_fail (core && core->anal && fcn);
	if (!core->anal->esil) {
		eprintf ("Please run aeim\n");
		return;
	}
	RAnal *anal = core->anal;
	Sdb *TDB = anal->sdb_types;
	bool chk_constraint = r_config_get_i (core->config, "anal.types.constraint");
	int ret, bsize = R_MAX (64, core->blocksize);
	const int mininstrsz = r_anal_archinfo (anal, R_ANAL_ARCHINFO_MIN_OP_SIZE);
	const int minopcode = R_MAX (1, mininstrsz);
	int cur_idx , prev_idx = 0;
	RConfigHold *hc = r_config_hold_new (core->config);
	if (!hc) {
		return;
	}
	RDebugTrace *dt = NULL;
	RAnalEsilTrace *et = NULL;
	if (!anal_emul_init (core, hc, &dt, &et) || !fcn) {
		anal_emul_restore (core, hc, dt, et);
		return;
	}
	ut8 *buf = malloc (bsize);
	if (!buf) {
		anal_emul_restore (core, hc, dt, et);
		return;
	}
	Sdb *etracedb = core->anal->esil->trace->db;
	HtPPOptions opt = etracedb->ht->opt;
	ht_pp_free (etracedb->ht);
	etracedb->ht = ht_pp_new_size (fcn->ninstr * 0xf, opt.dupvalue, opt.freefn, opt.calcsizeV);
	etracedb->ht->opt = opt;
	RDebugTrace *dtrace = core->dbg->trace;
	opt = dtrace->ht->opt;
	ht_pp_free (dtrace->ht);
	dtrace->ht = ht_pp_new_size (fcn->ninstr, opt.dupvalue, opt.freefn, opt.calcsizeV);
	dtrace->ht->opt = opt;
	char *fcn_name = NULL;
	char *ret_type = NULL;
	bool str_flag = false;
	bool prop = false;
	bool prev_var = false;
	char prev_type[256] = {0};
	const char *prev_dest = NULL;
	char *ret_reg = NULL;
	const char *pc = r_reg_get_name (core->dbg->reg, R_REG_NAME_PC);
	if (!pc) {
        free (buf);
		return;
	}
	RRegItem *r = r_reg_get (core->dbg->reg, pc, -1);
	if (!r) {
		free (buf);
		return;
	}
	r_cons_break_push (NULL, NULL);
	r_list_sort (fcn->bbs, bb_cmpaddr);  
	r_list_foreach (fcn->bbs, it, bb) {
		ut64 addr = bb->addr;
		int i = 0;
		r_reg_set_value (core->dbg->reg, r, addr);
		while (1) {
			if (r_cons_is_breaked ()) {
				goto out_function;
			}
			if (i >= (bsize - 32)) {
				i = 0;
			}
			ut64 pcval = r_reg_getv (anal->reg, pc);
			if ((addr >= bb->addr + bb->size) || (addr < bb->addr) || pcval != addr) {
				break;
			}
			if (!i) {
				r_io_read_at (core->io, addr, buf, bsize);
			}
			ret = r_anal_op (anal, &aop, addr, buf + i, bsize - i, R_ANAL_OP_MASK_BASIC | R_ANAL_OP_MASK_VAL);
			if (ret <= 0) {
				i += minopcode;
				addr += minopcode;
				r_anal_op_fini (&aop);
				continue;
			}
			int loop_count = sdb_num_get (anal->esil->trace->db, sdb_fmt ("0x%"PFMT64x".count", addr), 0);
			if (loop_count > LOOP_MAX || aop.type == R_ANAL_OP_TYPE_RET) {
				r_anal_op_fini (&aop);
				break;
			}
			sdb_num_set (anal->esil->trace->db, sdb_fmt ("0x%"PFMT64x".count", addr), loop_count + 1, 0);
			if (r_anal_op_nonlinear (aop.type)) {    
				r_reg_set_value (core->dbg->reg, r, addr + ret);
			} else {
				r_core_esil_step (core, UT64_MAX, NULL, NULL, false);
			}
			bool userfnc = false;
			Sdb *trace = anal->esil->trace->db;
			cur_idx = sdb_num_get (trace, "idx", 0);
			RAnalVar *var = r_anal_get_used_function_var (anal, aop.addr);
			RAnalOp *next_op = r_core_anal_op (core, addr + ret, R_ANAL_OP_MASK_BASIC);  
			ut32 type = aop.type & R_ANAL_OP_TYPE_MASK;
			if (aop.type == R_ANAL_OP_TYPE_CALL || aop.type & R_ANAL_OP_TYPE_UCALL) {
				char *full_name = NULL;
				ut64 callee_addr;
				if (aop.type == R_ANAL_OP_TYPE_CALL) {
					RAnalFunction *fcn_call = r_anal_get_fcn_in (anal, aop.jump, -1);
					if (fcn_call) {
						full_name = fcn_call->name;
						callee_addr = fcn_call->addr;
					}
				} else if (aop.ptr != UT64_MAX) {
					RFlagItem *flag = r_flag_get_by_spaces (core->flags, aop.ptr, R_FLAGS_FS_IMPORTS, NULL);
					if (flag && flag->realname) {
						full_name = flag->realname;
						callee_addr = aop.ptr;
					}
				}
				if (full_name) {
					if (r_type_func_exist (TDB, full_name)) {
						fcn_name = strdup (full_name);
					} else {
						fcn_name = r_type_func_guess (TDB, full_name);
					}
					if (!fcn_name) {
						fcn_name = strdup (full_name);
						userfnc = true;
					}
					const char* Cc = r_anal_cc_func (anal, fcn_name);
					if (Cc && r_anal_cc_exist (anal, Cc)) {
						char *cc = strdup (Cc);
						type_match (core, fcn_name, addr, bb->addr, cc, prev_idx, userfnc, callee_addr);
						prev_idx = cur_idx;
						R_FREE (ret_type);
						const char *rt = r_type_func_ret (TDB, fcn_name);
						if (rt) {
							ret_type = strdup (rt);
						}
						R_FREE (ret_reg);
						const char *rr = r_anal_cc_ret (anal, cc);
						if (rr) {
							ret_reg = strdup (rr);
						}
						resolved = false;
						free (cc);
					}
					if (!strcmp (fcn_name, "__stack_chk_fail")) {
						const char *query = sdb_fmt ("%d.addr", cur_idx - 1);
						ut64 mov_addr = sdb_num_get (trace, query, 0);
						RAnalOp *mop = r_core_anal_op (core, mov_addr, R_ANAL_OP_MASK_VAL | R_ANAL_OP_MASK_BASIC);
						if (mop) {
							RAnalVar *mopvar = r_anal_get_used_function_var (anal, mop->addr);
							ut32 type = mop->type & R_ANAL_OP_TYPE_MASK;
							if (type == R_ANAL_OP_TYPE_MOV) {
								__var_rename (anal, mopvar, "canary", addr);
							}
						}
						r_anal_op_free (mop);
					}
					free (fcn_name);
				}
			} else if (!resolved && ret_type && ret_reg) {
				char src[REGNAME_SIZE] = {0};
				const char *query = sdb_fmt ("%d.reg.write", cur_idx);
				const char *cur_dest = sdb_const_get (trace, query, 0);
				get_src_regname (core, aop.addr, src, sizeof (src));
				if (ret_reg && *src && strstr (ret_reg, src)) {
					if (var && aop.direction == R_ANAL_OP_DIR_WRITE) {
						__var_retype (anal, var, NULL, ret_type, false, false);
						resolved = true;
					} else if (type == R_ANAL_OP_TYPE_MOV) {
						R_FREE (ret_reg);
						if (cur_dest) {
							ret_reg = strdup (cur_dest);
						}
					}
				} else if (cur_dest) {
					char *foo = strdup (cur_dest);
					char *tmp = strchr (foo, ',');
					if (tmp) {
						*tmp++ = '\0';
					}
					if (ret_reg && (strstr (ret_reg, foo) || (tmp && strstr (ret_reg, tmp)))) {
						resolved = true;
					} else if (type == R_ANAL_OP_TYPE_MOV &&
							(next_op && next_op->type == R_ANAL_OP_TYPE_MOV)){
						char nsrc[REGNAME_SIZE] = {0};
						get_src_regname (core, next_op->addr, nsrc, sizeof (nsrc));
						if (ret_reg && *nsrc && strstr (ret_reg, nsrc) && var &&
								aop.direction == R_ANAL_OP_DIR_READ) {
							__var_retype (anal, var, NULL, ret_type, true, false);
						}
					}
					free (foo);
				}
			}
			if (var) {
				bool sign = false;
				if ((type == R_ANAL_OP_TYPE_CMP) && next_op) {
					if (next_op->sign) {
						sign = true;
					} else {
						__var_retype (anal, var, NULL, "unsigned", false, true);
					}
				}
				if (sign || aop.sign) {
					__var_retype (anal, var, NULL, "signed", false, true);
				}
				if (prev_dest && (type == R_ANAL_OP_TYPE_MOV || type == R_ANAL_OP_TYPE_STORE)) {
					char reg[REGNAME_SIZE] = {0};
					get_src_regname (core, addr, reg, sizeof (reg));
					bool match = strstr (prev_dest, reg) != NULL;
					if (str_flag && match) {
						__var_retype (anal, var, NULL, "const char *", false, false);
					}
					if (prop && match && prev_var) {
						__var_retype (anal, var, NULL, prev_type, false, false);
					}
				}
				if (chk_constraint && var && (type == R_ANAL_OP_TYPE_CMP && aop.disp != UT64_MAX)
						&& next_op && next_op->type == R_ANAL_OP_TYPE_CJMP) {
					bool jmp = false;
					RAnalOp *jmp_op = {0};
					ut64 jmp_addr = next_op->jump;
					RAnalBlock *jmpbb = r_anal_fcn_bbget_in (anal, fcn, jmp_addr);
					for (i = 0; i < MAX_INSTR ; i++) {
						jmp_op = r_core_anal_op (core, jmp_addr, R_ANAL_OP_MASK_BASIC);
						if (!jmp_op) {
							break;
						}
						if ((jmp_op->type == R_ANAL_OP_TYPE_RET && r_anal_block_contains (jmpbb, jmp_addr))
								|| jmp_op->type == R_ANAL_OP_TYPE_CJMP) {
							jmp = true;
							r_anal_op_free (jmp_op);
							break;
						}
						jmp_addr += jmp_op->size;
						r_anal_op_free (jmp_op);
					}
					RAnalVarConstraint constr = {
						.cond = jmp? cond_invert (anal, next_op->cond): next_op->cond,
						.val = aop.val
					};
					r_anal_var_add_constraint (var, &constr);
				}
			}
			prev_var = (var && aop.direction == R_ANAL_OP_DIR_READ);
			str_flag = false;
			prop = false;
			prev_dest = NULL;
			switch (type) {
			case R_ANAL_OP_TYPE_MOV:
			case R_ANAL_OP_TYPE_LEA:
			case R_ANAL_OP_TYPE_LOAD:
				if (aop.ptr && aop.refptr && aop.ptr != UT64_MAX) {
					if (type == R_ANAL_OP_TYPE_LOAD) {
						ut8 buf[256] = {0};
						r_io_read_at (core->io, aop.ptr, buf, sizeof (buf) - 1);
						ut64 ptr = r_read_ble (buf, core->print->big_endian, aop.refptr * 8);
						if (ptr && ptr != UT64_MAX) {
							RFlagItem *f = r_flag_get_by_spaces (core->flags, ptr, R_FLAGS_FS_STRINGS, NULL);
							if (f) {
								str_flag = true;
							}
						}
					} else if (r_flag_exist_at (core->flags, "str", 3, aop.ptr)) {
						str_flag = true;
					}
				}
				if (var && str_flag) {
					__var_retype (anal, var, NULL, "const char *", false, false);
				}
				const char *query = sdb_fmt ("%d.reg.write", cur_idx);
				prev_dest = sdb_const_get (trace, query, 0);
				if (var) {
					strncpy (prev_type, var->type, sizeof (prev_type) - 1);
					prop = true;
				}
			}
			i += ret;
			addr += ret;
			r_anal_op_free (next_op);
			r_anal_op_fini (&aop);
		}
	}
	RList *list = r_anal_var_list (anal, fcn, R_ANAL_VAR_KIND_REG);
	RAnalVar *rvar;
	RListIter *iter;
	r_list_foreach (list, iter, rvar) {
		RAnalVar *lvar = r_anal_var_get_dst_var (rvar);
		RRegItem *i = r_reg_index_get (anal->reg, rvar->delta);
		if (!i) {
			continue;
		}
		if (lvar) {
			__var_retype (anal, rvar, NULL, lvar->type, false, false);
			__var_retype (anal, lvar, NULL, rvar->type, false, false);
		}
	}
	r_list_free (list);
out_function:
	R_FREE (ret_reg);
	R_FREE (ret_type);
	free (buf);
	r_cons_break_pop();
	anal_emul_restore (core, hc, dt, et);
}