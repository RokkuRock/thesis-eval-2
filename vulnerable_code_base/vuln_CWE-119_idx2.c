static void coerce_reg_to_32(struct bpf_reg_state *reg)
{
	reg->var_off = tnum_cast(reg->var_off, 4);
	__update_reg_bounds(reg);
}