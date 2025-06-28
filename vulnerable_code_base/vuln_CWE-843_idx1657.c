static int sanitize_ptr_alu(struct bpf_verifier_env *env,
			    struct bpf_insn *insn,
			    const struct bpf_reg_state *ptr_reg,
			    const struct bpf_reg_state *off_reg,
			    struct bpf_reg_state *dst_reg,
			    struct bpf_sanitize_info *info,
			    const bool commit_window)
{
	struct bpf_insn_aux_data *aux = commit_window ? cur_aux(env) : &info->aux;
	struct bpf_verifier_state *vstate = env->cur_state;
	bool off_is_imm = tnum_is_const(off_reg->var_off);
	bool off_is_neg = off_reg->smin_value < 0;
	bool ptr_is_dst_reg = ptr_reg == dst_reg;
	u8 opcode = BPF_OP(insn->code);
	u32 alu_state, alu_limit;
	struct bpf_reg_state tmp;
	bool ret;
	int err;
	if (can_skip_alu_sanitation(env, insn))
		return 0;
	if (vstate->speculative)
		goto do_sim;
	if (!commit_window) {
		if (!tnum_is_const(off_reg->var_off) &&
		    (off_reg->smin_value < 0) != (off_reg->smax_value < 0))
			return REASON_BOUNDS;
		info->mask_to_left = (opcode == BPF_ADD &&  off_is_neg) ||
				     (opcode == BPF_SUB && !off_is_neg);
	}
	err = retrieve_ptr_limit(ptr_reg, &alu_limit, info->mask_to_left);
	if (err < 0)
		return err;
	if (commit_window) {
		alu_state = info->aux.alu_state;
		alu_limit = abs(info->aux.alu_limit - alu_limit);
	} else {
		alu_state  = off_is_neg ? BPF_ALU_NEG_VALUE : 0;
		alu_state |= off_is_imm ? BPF_ALU_IMMEDIATE : 0;
		alu_state |= ptr_is_dst_reg ?
			     BPF_ALU_SANITIZE_SRC : BPF_ALU_SANITIZE_DST;
	}
	err = update_alu_sanitation_state(aux, alu_state, alu_limit);
	if (err < 0)
		return err;
do_sim:
	if (commit_window || off_is_imm)
		return 0;
	if (!ptr_is_dst_reg) {
		tmp = *dst_reg;
		*dst_reg = *ptr_reg;
	}
	ret = push_stack(env, env->insn_idx + 1, env->insn_idx, true);
	if (!ptr_is_dst_reg && ret)
		*dst_reg = tmp;
	return !ret ? REASON_STACK : 0;
}