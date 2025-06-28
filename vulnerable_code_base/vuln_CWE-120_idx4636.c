struct bpf_prog *bpf_patch_insn_single(struct bpf_prog *prog, u32 off,
				       const struct bpf_insn *patch, u32 len)
{
	u32 insn_adj_cnt, insn_rest, insn_delta = len - 1;
	struct bpf_prog *prog_adj;
	if (insn_delta == 0) {
		memcpy(prog->insnsi + off, patch, sizeof(*patch));
		return prog;
	}
	insn_adj_cnt = prog->len + insn_delta;
	prog_adj = bpf_prog_realloc(prog, bpf_prog_size(insn_adj_cnt),
				    GFP_USER);
	if (!prog_adj)
		return NULL;
	prog_adj->len = insn_adj_cnt;
	insn_rest = insn_adj_cnt - off - len;
	memmove(prog_adj->insnsi + off + len, prog_adj->insnsi + off + 1,
		sizeof(*patch) * insn_rest);
	memcpy(prog_adj->insnsi + off, patch, sizeof(*patch) * len);
	bpf_adj_branches(prog_adj, off, insn_delta);
	return prog_adj;
}