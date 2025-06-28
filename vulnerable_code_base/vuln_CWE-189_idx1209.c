static int sanitize_ptr_alu(struct bpf_verifier_env *env,
			    struct bpf_insn *insn,
			    const struct bpf_reg_state *ptr_reg,
			    struct bpf_reg_state *dst_reg,
			    bool off_is_neg)
{
	struct bpf_verifier_state *vstate = env->cur_state;
	struct bpf_insn_aux_data *aux = cur_aux(env);
	bool ptr_is_dst_reg = ptr_reg == dst_reg;
	u8 opcode = BPF_OP(insn->code);
	u32 alu_state, alu_limit;
	struct bpf_reg_state tmp;
	bool ret;
	if (env->allow_ptr_leaks || BPF_SRC(insn->code) == BPF_K)
		return 0;
	if (vstate->speculative)
		goto do_sim;
	alu_state  = off_is_neg ? BPF_ALU_NEG_VALUE : 0;
	alu_state |= ptr_is_dst_reg ?
		     BPF_ALU_SANITIZE_SRC : BPF_ALU_SANITIZE_DST;
	if (retrieve_ptr_limit(ptr_reg, &alu_limit, opcode, off_is_neg))
		return 0;
	if (aux->alu_state &&
	    (aux->alu_state != alu_state ||
	     aux->alu_limit != alu_limit))
		return -EACCES;
	aux->alu_state = alu_state;
	aux->alu_limit = alu_limit;
do_sim:
	if (!ptr_is_dst_reg) {
		tmp = *dst_reg;
		*dst_reg = *ptr_reg;
	}
	ret = push_stack(env, env->insn_idx + 1, env->insn_idx, true);
	if (!ptr_is_dst_reg)
		*dst_reg = tmp;
	return !ret ? -EFAULT : 0;
}