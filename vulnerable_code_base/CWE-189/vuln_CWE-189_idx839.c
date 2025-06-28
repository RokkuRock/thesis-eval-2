static int check_cond_jmp_op(struct bpf_verifier_env *env,
			     struct bpf_insn *insn, int *insn_idx)
{
	struct bpf_verifier_state *this_branch = env->cur_state;
	struct bpf_verifier_state *other_branch;
	struct bpf_reg_state *regs = this_branch->frame[this_branch->curframe]->regs;
	struct bpf_reg_state *dst_reg, *other_branch_regs;
	u8 opcode = BPF_OP(insn->code);
	int err;
	if (opcode > BPF_JSLE) {
		verbose(env, "invalid BPF_JMP opcode %x\n", opcode);
		return -EINVAL;
	}
	if (BPF_SRC(insn->code) == BPF_X) {
		if (insn->imm != 0) {
			verbose(env, "BPF_JMP uses reserved fields\n");
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
	} else {
		if (insn->src_reg != BPF_REG_0) {
			verbose(env, "BPF_JMP uses reserved fields\n");
			return -EINVAL;
		}
	}
	err = check_reg_arg(env, insn->dst_reg, SRC_OP);
	if (err)
		return err;
	dst_reg = &regs[insn->dst_reg];
	if (BPF_SRC(insn->code) == BPF_K) {
		int pred = is_branch_taken(dst_reg, insn->imm, opcode);
		if (pred == 1) {
			*insn_idx += insn->off;
			return 0;
		} else if (pred == 0) {
			return 0;
		}
	}
	other_branch = push_stack(env, *insn_idx + insn->off + 1, *insn_idx);
	if (!other_branch)
		return -EFAULT;
	other_branch_regs = other_branch->frame[other_branch->curframe]->regs;
	if (BPF_SRC(insn->code) == BPF_X) {
		if (dst_reg->type == SCALAR_VALUE &&
		    regs[insn->src_reg].type == SCALAR_VALUE) {
			if (tnum_is_const(regs[insn->src_reg].var_off))
				reg_set_min_max(&other_branch_regs[insn->dst_reg],
						dst_reg, regs[insn->src_reg].var_off.value,
						opcode);
			else if (tnum_is_const(dst_reg->var_off))
				reg_set_min_max_inv(&other_branch_regs[insn->src_reg],
						    &regs[insn->src_reg],
						    dst_reg->var_off.value, opcode);
			else if (opcode == BPF_JEQ || opcode == BPF_JNE)
				reg_combine_min_max(&other_branch_regs[insn->src_reg],
						    &other_branch_regs[insn->dst_reg],
						    &regs[insn->src_reg],
						    &regs[insn->dst_reg], opcode);
		}
	} else if (dst_reg->type == SCALAR_VALUE) {
		reg_set_min_max(&other_branch_regs[insn->dst_reg],
					dst_reg, insn->imm, opcode);
	}
	if (BPF_SRC(insn->code) == BPF_K &&
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
	if (env->log.level)
		print_verifier_state(env, this_branch->frame[this_branch->curframe]);
	return 0;
}