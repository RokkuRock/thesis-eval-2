static bool decode(RArchSession *as, RAnalOp *op, RArchDecodeMask mask) {
	int len = op->size;
	const ut8 *_buf = op->bytes;
	const ut64 addr = op->addr;
	if (len < 1) {
		return false;
	}
	ut8 *buf = (ut8*)_buf;  
	ut64 dst = 0LL;
	if (!op) {
		return 1;
	}
	if (mask & R_ARCH_OP_MASK_DISASM) {
		(void) disassemble (op, buf, len);
	}
	r_strbuf_init (&op->esil);
	op->size = 1;
	op->id = getid (buf[0]);
	switch (buf[0]) {
	case '[':
		op->type = R_ANAL_OP_TYPE_CJMP;
		op->jump = dst;
		op->fail = addr + 1;
		RArch *a = as->arch;
		RIOReadAt read_at = NULL;
		RBin *bin = R_UNWRAP2 (a, binb.bin);
		if (bin && bin->iob.read_at) {
			RIOReadAt read_at = bin->iob.read_at;
			buf = malloc (0xff);
			read_at (bin->iob.io, op->addr, buf, 0xff);
		}
		r_strbuf_set (&op->esil, "1,pc,-,brk,=[4],4,brk,+=");
#if 1
		{
			const ut8 *p = buf + 1;
			int lev = 0, i = 1;
			len--;
			while (i < len && *p) {
				if (*p == '[') {
					lev++;
				}
				if (*p == ']') {
					lev--;
					if (lev == -1) {
						dst = addr + (size_t)(p - buf) + 1;
						op->jump = dst;
						r_strbuf_set (&op->esil, "1,pc,-,brk,=[4],4,brk,+=,");
						goto beach;
					}
				}
				if (*p == 0x00 || *p == 0xff) {
					op->type = R_ANAL_OP_TYPE_ILL;
					goto beach;
				}
				if (read_at && i == len - 1) {
					break;
					int new_buf_len = len + 1 + BUFSIZE_INC;
					ut8 *new_buf = calloc (new_buf_len, 1);
					if (new_buf) {
						free (buf);
						memcpy (new_buf, op->bytes, new_buf_len);
						buf = new_buf;
						read_at (bin->iob.io, op->addr + i, buf + i, 0xff);
						p = buf + i;
						len += BUFSIZE_INC;
					}
				}
				p++;
				i++;
			}
		}
beach:
		free (buf);
#endif
		break;
	case ']':
		op->type = R_ANAL_OP_TYPE_UJMP;
		r_strbuf_set (&op->esil, "4,brk,-=,ptr,[1],?{,brk,[4],pc,=,}");
		break;
	case '>':
		op->type = R_ANAL_OP_TYPE_ADD;
		op->size = countChar (buf, len, '>');
		r_strbuf_setf (&op->esil, "%d,ptr,+=", op->size);
		break;
	case '<':
		op->type = R_ANAL_OP_TYPE_SUB;
		op->size = countChar (buf, len, '<');
		r_strbuf_setf (&op->esil, "%d,ptr,-=", op->size);
		break;
	case '+':
		op->size = countChar (buf, len, '+');
		op->type = R_ANAL_OP_TYPE_ADD;
		r_strbuf_setf (&op->esil, "%d,ptr,+=[1]", op->size);
		break;
	case '-':
		op->type = R_ANAL_OP_TYPE_SUB;
		op->size = countChar (buf, len, '-');
		r_strbuf_setf (&op->esil, "%d,ptr,-=[1]", op->size);
		break;
	case '.':
		op->type = R_ANAL_OP_TYPE_STORE;
		r_strbuf_set (&op->esil, "ptr,[1],scr,=[1],1,scr,+=");
		break;
	case ',':
		op->type = R_ANAL_OP_TYPE_LOAD;
		r_strbuf_set (&op->esil, "kbd,[1],ptr,=[1],1,kbd,+=");
		break;
	case 0x00:
	case 0xff:
		op->type = R_ANAL_OP_TYPE_TRAP;
		break;
	default:
		op->type = R_ANAL_OP_TYPE_NOP;
		r_strbuf_set (&op->esil, ",");
		break;
	}
	return op->size;
}