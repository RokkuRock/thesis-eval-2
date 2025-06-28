static int hexagon_v6_op(RAnal *anal, RAnalOp *op, ut64 addr, const ut8 *buf, int len, RAnalOpMask mask) {
	HexInsn hi = {0};;
	ut32 data = 0;
	data = r_read_le32 (buf);
	int size = hexagon_disasm_instruction (data, &hi, (ut32) addr);
	op->size = size;
	if (size <= 0) {
		return size;
	}
	op->addr = addr;
	return hexagon_anal_instruction (&hi, op);
}