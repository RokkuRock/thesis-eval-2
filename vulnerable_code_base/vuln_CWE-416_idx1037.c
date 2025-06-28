R_API bool r_reg_set_value(RReg *reg, RRegItem *item, ut64 value) {
	ut8 bytes[12];
	ut8 *src = bytes;
	r_return_val_if_fail (reg && item, false);
	if (r_reg_is_readonly (reg, item)) {
		return true;
	}
	if (item->offset < 0) {
		return true;
	}
	RRegArena *arena = reg->regset[item->arena].arena;
	if (!arena) {
		return false;
	}
	switch (item->size) {
	case 80:
	case 96:  
		r_reg_set_longdouble (reg, item, (long double)value);
		break;
	case 64:
		if (reg->big_endian) {
			r_write_be64 (src, value);
		} else {
			r_write_le64 (src, value);
		}
		break;
	case 32:
		if (reg->big_endian) {
			r_write_be32 (src, value);
		} else {
			r_write_le32 (src, value);
		}
		break;
	case 16:
		if (reg->big_endian) {
			r_write_be16 (src, value);
		} else {
			r_write_le16 (src, value);
		}
		break;
	case 8:
		r_write_ble8 (src, (ut8) (value & UT8_MAX));
		break;
	case 1:
		if (value) {
			ut8 *buf = arena->bytes + (item->offset / 8);
			int bit = (item->offset % 8);
			ut8 mask = (1 << bit);
			buf[0] = (buf[0] & (0xff ^ mask)) | mask;
		} else {
			int idx = item->offset / 8;
			if (idx + item->size > arena->size) {
				eprintf ("RRegSetOverflow %d vs %d\n", idx + item->size, arena->size);
				return false;
			}
			ut8 *buf = arena->bytes + idx;
			int bit = item->offset % 8;
			ut8 mask = 0xff ^ (1 << bit);
			buf[0] = (buf[0] & mask) | 0;
		}
		return true;
	case 128:
	case 256:
		return false;  
	default:
		eprintf ("r_reg_set_value: Bit size %d not supported\n", item->size);
		return false;
	}
	const bool fits_in_arena = (arena->size - BITS2BYTES (item->offset) - BITS2BYTES (item->size)) >= 0;
	if (src && fits_in_arena) {
		r_mem_copybits (reg->regset[item->arena].arena->bytes +
				BITS2BYTES (item->offset),
				src, item->size);
		return true;
	}
	eprintf ("r_reg_set_value: Cannot set %s to 0x%" PFMT64x "\n", item->name, value);
	return false;
}