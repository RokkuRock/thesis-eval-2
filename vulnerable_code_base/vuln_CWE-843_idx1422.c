static int check_cond_jmp_op(struct bpf_verifier_env *env,
			     struct bpf_insn *insn, int *insn_idx)
{
	struct bpf_verifier_state *this_branch = env->cur_state;
	struct bpf_verifier_state *other_branch;
	struct bpf_reg_state *regs = this_branch->frame[this_branch->curframe]->regs;
	struct bpf_reg_state *dst_reg, *other_branch_regs, *src_reg = NULL;
	u8 opcode = BPF_OP(insn->code);
	bool is_jmp32;
	int pred = -1;
	int err;
	if (opcode == BPF_JA || opcode > BPF_JSLE) {
		verbose(env, "invalid BPF_JMP/JMP32 opcode %x\n", opcode);
		return -EINVAL;
	}
	if (BPF_SRC(insn->code) == BPF_X) {
		if (insn->imm != 0) {
			verbose(env, "BPF_JMP/JMP32 uses reserved fields\n");
			return -EINVAL;
		}
		err = check_reg_arg(env, insn->src_reg, SRC_OP);
		if (err)
			return err;
		if (is_pointer_value(env, insn->src_reg)) {
			verbose(env, "R%d pointer comparison prohibited\n",
				insn->src_reg);
			return -EACCES;
		}
		src_reg = &regs[insn->src_reg];
	} else {
		if (insn->src_reg != BPF_REG_0) {
			verbose(env, "BPF_JMP/JMP32 uses reserved fields\n");
			return -EINVAL;
		}
	}
	err = check_reg_arg(env, insn->dst_reg, SRC_OP);
	if (err)
		return err;
	dst_reg = &regs[insn->dst_reg];
	is_jmp32 = BPF_CLASS(insn->code) == BPF_JMP32;
	if (BPF_SRC(insn->code) == BPF_K) {
		pred = is_branch_taken(dst_reg, insn->imm, opcode, is_jmp32);
	} else if (src_reg->type == SCALAR_VALUE &&
		   is_jmp32 && tnum_is_const(tnum_subreg(src_reg->var_off))) {
		pred = is_branch_taken(dst_reg,
				       tnum_subreg(src_reg->var_off).value,
				       opcode,
				       is_jmp32);
	} else if (src_reg->type == SCALAR_VALUE &&
		   !is_jmp32 && tnum_is_const(src_reg->var_off)) {
		pred = is_branch_taken(dst_reg,
				       src_reg->var_off.value,
				       opcode,
				       is_jmp32);
	} else if (reg_is_pkt_pointer_any(dst_reg) &&
		   reg_is_pkt_pointer_any(src_reg) &&
		   !is_jmp32) {
		pred = is_pkt_ptr_branch_taken(dst_reg, src_reg, opcode);
	}
	if (pred >= 0) {
		if (!__is_pointer_value(false, dst_reg))
			err = mark_chain_precision(env, insn->dst_reg);
		if (BPF_SRC(insn->code) == BPF_X && !err &&
		    !__is_pointer_value(false, src_reg))
			err = mark_chain_precision(env, insn->src_reg);
		if (err)
			return err;
	}
	if (pred == 1) {
		*insn_idx += insn->off;
		return 0;
	} else if (pred == 0) {
		return 0;
	}
	other_branch = push_stack(env, *insn_idx + insn->off + 1, *insn_idx,
				  false);
	if (!other_branch)
		return -EFAULT;
	other_branch_regs = other_branch->frame[other_branch->curframe]->regs;
	if (BPF_SRC(insn->code) == BPF_X) {
		struct bpf_reg_state *src_reg = &regs[insn->src_reg];
		if (dst_reg->type == SCALAR_VALUE &&
		    src_reg->type == SCALAR_VALUE) {
			if (tnum_is_const(src_reg->var_off) ||
			    (is_jmp32 &&
			     tnum_is_const(tnum_subreg(src_reg->var_off))))
				reg_set_min_max(&other_branch_regs[insn->dst_reg],
						dst_reg,
						src_reg->var_off.value,
						tnum_subreg(src_reg->var_off).value,
						opcode, is_jmp32);
			else if (tnum_is_const(dst_reg->var_off) ||
				 (is_jmp32 &&
				  tnum_is_const(tnum_subreg(dst_reg->var_off))))
				reg_set_min_max_inv(&other_branch_regs[insn->src_reg],
						    src_reg,
						    dst_reg->var_off.value,
						    tnum_subreg(dst_reg->var_off).value,
						    opcode, is_jmp32);
			else if (!is_jmp32 &&
				 (opcode == BPF_JEQ || opcode == BPF_JNE))
				reg_combine_min_max(&other_branch_regs[insn->src_reg],
						    &other_branch_regs[insn->dst_reg],
						    src_reg, dst_reg, opcode);
			if (src_reg->id &&
			    !WARN_ON_ONCE(src_reg->id != other_branch_regs[insn->src_reg].id)) {
				find_equal_scalars(this_branch, src_reg);
				find_equal_scalars(other_branch, &other_branch_regs[insn->src_reg]);
			}
		}
	} else if (dst_reg->type == SCALAR_VALUE) {
		reg_set_min_max(&other_branch_regs[insn->dst_reg],
					dst_reg, insn->imm, (u32)insn->imm,
					opcode, is_jmp32);
	}
	if (dst_reg->type == SCALAR_VALUE && dst_reg->id &&
	    !WARN_ON_ONCE(dst_reg->id != other_branch_regs[insn->dst_reg].id)) {
		find_equal_scalars(this_branch, dst_reg);
		find_equal_scalars(other_branch, &other_branch_regs[insn->dst_reg]);
	}
	if (!is_jmp32 && BPF_SRC(insn->code) == BPF_K &&
	    insn->imm == 0 && (opcode == BPF_JEQ || opcode == BPF_JNE) &&
	    reg_type_may_be_null(dst_reg->type)) {
		mark_ptr_or_null_regs(this_branch, insn->dst_reg,
				      opcode == BPF_JNE);
		mark_ptr_or_null_regs(other_branch, insn->dst_reg,
				      opcode == BPF_JEQ);
	} else if (!try_match_pkt_pointers(insn, dst_reg, &regs[insn->src_reg],
					   this_branch, other_branch) &&
		   is_pointer_value(env, insn->dst_reg)) {
		verbose(env, "R%d pointer comparison prohibited\n",
			insn->dst_reg);
		return -EACCES;
	}
	if (env->log.level & BPF_LOG_LEVEL)
		print_verifier_state(env, this_branch->frame[this_branch->curframe]);
	return 0;
}