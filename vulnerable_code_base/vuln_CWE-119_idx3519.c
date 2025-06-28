static void vector64_dst_append(RStrBuf *sb, csh *handle, cs_insn *insn, int n, int i) {
	cs_arm64_op op = INSOP64 (n);
	if (op.vector_index != -1) {
		i = op.vector_index;
	}
#if CS_API_MAJOR == 4
	const bool isvessas = (op.vess || op.vas);
#else
	const bool isvessas = op.vas;
#endif
	if (isvessas && i != -1) {
		int size = vector_size (&op);
		int shift = i * size;
		char *regc = "l";
		size_t s = sizeof (bitmask_by_width) / sizeof (*bitmask_by_width);
		size_t index = size > 0? (size - 1) % s: 0;
		if (index >= BITMASK_BY_WIDTH_COUNT) {
			index = 0;
		}
		ut64 mask = bitmask_by_width[index];
		if (shift >= 64) {
			shift -= 64;
			regc = "h";
		}
		if (shift > 0 && shift < 64) {
			r_strbuf_appendf (sb, "%d,SWAP,0x%"PFMT64x",&,<<,%s%s,0x%"PFMT64x",&,|,%s%s",
				shift, mask, REG64 (n), regc, VEC64_MASK (shift, size), REG64 (n), regc);
		} else {
			int dimsize = size % 64;
			r_strbuf_appendf (sb, "0x%"PFMT64x",&,%s%s,0x%"PFMT64x",&,|,%s%s",
				mask, REG64 (n), regc, VEC64_MASK (shift, dimsize), REG64 (n), regc);
		}
	} else {
		r_strbuf_appendf (sb, "%s", REG64 (n));
	}
}