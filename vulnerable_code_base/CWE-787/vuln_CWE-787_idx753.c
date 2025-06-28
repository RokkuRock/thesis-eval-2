static int vax_op(RAnal *anal, RAnalOp *op, ut64 addr, const ut8 *buf, int len, RAnalOpMask mask) {
	op->size = 1;
	if (len < 1) {
		return -1;
	}
	op->addr = addr;
	op->type = R_ANAL_OP_TYPE_UNK;
	switch (buf[0]) {
	case 0x04:
		op->type = R_ANAL_OP_TYPE_RET;
		break;
	case 0x2e:
		op->type = R_ANAL_OP_TYPE_MOV;
		op->size = 8;
		break;
	case 0x78:
		op->type = R_ANAL_OP_TYPE_SHL;
		op->size = 8;
		break;
	case 0xc0:
	case 0xc1:
	case 0xd8:
		op->type = R_ANAL_OP_TYPE_ADD;
		op->size = 8;
		break;
	case 0xd7:
		op->type = R_ANAL_OP_TYPE_SUB;  
		op->size = 2;
		break;
	case 0x00:
	case 0x01:
		op->size = 1;
		op->type = R_ANAL_OP_TYPE_NOP;
		break;
	case 0xac:
		op->type = R_ANAL_OP_TYPE_XOR;
		op->size = 4;
		break;
	case 0x5a:
		op->size = 2;
		break;
	case 0x11:
	case 0x12:
	case 0x13:
	case 0x14:
	case 0x15:
	case 0x16:
	case 0x17:
	case 0x18:
	case 0x19:
	case 0x1e:
		op->size = 2;
		op->type = R_ANAL_OP_TYPE_CJMP;
		op->jump = op->addr + op->size + ((char)buf[1]);
		op->fail = op->addr + op->size;
		break;
	case 0xd0:  
		op->size = 7;
		op->type = R_ANAL_OP_TYPE_MOV;
		break;
	case 0xd4:  
		op->size = 3;
		op->type = R_ANAL_OP_TYPE_NOP;
		break;
	case 0xc2:  
		op->size = 3;
		op->type = R_ANAL_OP_TYPE_SUB;
		break;
	case 0xca:  
		op->size = 3;
		op->type = R_ANAL_OP_TYPE_SUB;
		break;
	case 0x31:
	case 0xe9:
		op->size = 3;
		op->type = R_ANAL_OP_TYPE_CJMP;
		if (len > 2) {
			op->jump = op->addr + op->size + ((buf[1] << 8) + buf[2]);
			op->fail = op->addr + op->size;
		}
		break;
	case 0xc6:
	case 0xc7:
		op->size = 8;
		op->type = R_ANAL_OP_TYPE_DIV;
		break;
	case 0x94:  
	case 0x7d:  
		op->size = 3;
		op->type = R_ANAL_OP_TYPE_MOV;
		break;
	case 0x90:
	case 0x9e:
	case 0xde:
		op->size = 7;
		op->type = R_ANAL_OP_TYPE_MOV;
		break;
	case 0xdd:
	case 0x9f:
	case 0xdf:
		op->size = 6;
		op->type = R_ANAL_OP_TYPE_PUSH;
		break;
	case 0xd1:
	case 0xd5:
	case 0x91:
	case 0x51:
	case 0x73:
		op->type = R_ANAL_OP_TYPE_CMP;
		op->size = 3;
		break;
	case 0x95:  
		op->type = R_ANAL_OP_TYPE_CMP;
		op->size = 6;
		break;
	case 0xd6:
	case 0x61:
		op->size = 2;
		op->type = R_ANAL_OP_TYPE_ADD;
		break;
	case 0x40:
		op->size = 5;
		op->type = R_ANAL_OP_TYPE_ADD;
		break;
	case 0x9a:
		op->size = 4;
		op->type = R_ANAL_OP_TYPE_MOV;
		break;
	case 0x83:
		op->type = R_ANAL_OP_TYPE_SUB;
		op->size = 5;
		break;
	case 0x62:
		op->type = R_ANAL_OP_TYPE_SUB;
		break;
	case 0xfb:  
		op->type = R_ANAL_OP_TYPE_CALL;
		op->size = 7;
		{
			int oa = 3;
			ut32 delta = buf[oa];
			delta |= (ut32)(buf[oa + 1]) << 8;
			delta |= (ut32)(buf[oa + 2]) << 16;
			delta |= (ut32)(buf[oa + 3]) << 24;
			delta += op->size;
			op->jump = op->addr + delta;
		}
		op->fail = op->addr + op->size;
		break;
	case 0xff:
		op->size = 2;
		break;
	}
	return op->size;
}