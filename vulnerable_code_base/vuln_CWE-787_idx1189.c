static VariableLocation *parse_dwarf_location (Context *ctx, const RBinDwarfAttrValue *loc, const RBinDwarfAttrValue *frame_base) {
	if (loc->kind != DW_AT_KIND_BLOCK && loc->kind != DW_AT_KIND_LOCLISTPTR && loc->kind != DW_AT_KIND_REFERENCE && loc->kind != DW_AT_KIND_CONSTANT) {
		return NULL;
	}
	RBinDwarfBlock block;
	if (loc->kind == DW_AT_KIND_LOCLISTPTR || loc->kind == DW_AT_KIND_REFERENCE || loc->kind == DW_AT_KIND_CONSTANT) {
		ut64 offset = loc->reference;
		RBinDwarfLocList *range_list = ht_up_find (ctx->locations, offset, NULL);
		if (!range_list) {  
			return NULL;
		}
		RBinDwarfLocRange *range = find_largest_loc_range (range_list->list);
		if (!range) {
			return NULL;
		}
		block = *range->expression;
	} else {
		block = loc->block;
	}
	VariableLocationKind kind = LOCATION_UNKNOWN;
	st64 offset = 0;
	ut64 address = 0;
	ut64 reg_num = -1;
	const char *reg_name = NULL;  
	size_t i;
	for (i = 0; i < block.length; i++) {
		switch (block.data[i]) {
		case DW_OP_fbreg: {
			if (i == block.length - 1) {
				return NULL;
			}
			const ut8 *dump = &block.data[++i];
			offset = r_sleb128 (&dump, &block.data[loc->block.length]);
			if (frame_base) {
				VariableLocation *location = parse_dwarf_location (ctx, frame_base, NULL);
				if (location) {
					location->offset += offset;
					return location;
				}
				return NULL;
			} else {
				return NULL;
			}
			break;
		}
		case DW_OP_reg0:
		case DW_OP_reg1:
		case DW_OP_reg2:
		case DW_OP_reg3:
		case DW_OP_reg4:
		case DW_OP_reg5:
		case DW_OP_reg6:
		case DW_OP_reg7:
		case DW_OP_reg8:
		case DW_OP_reg9:
		case DW_OP_reg10:
		case DW_OP_reg11:
		case DW_OP_reg12:
		case DW_OP_reg13:
		case DW_OP_reg14:
		case DW_OP_reg15:
		case DW_OP_reg16:
		case DW_OP_reg17:
		case DW_OP_reg18:
		case DW_OP_reg19:
		case DW_OP_reg20:
		case DW_OP_reg21:
		case DW_OP_reg22:
		case DW_OP_reg23:
		case DW_OP_reg24:
		case DW_OP_reg25:
		case DW_OP_reg26:
		case DW_OP_reg27:
		case DW_OP_reg28:
		case DW_OP_reg29:
		case DW_OP_reg30:
		case DW_OP_reg31: {
			reg_num = block.data[i] - DW_OP_reg0;  
			reg_name = get_dwarf_reg_name (ctx->anal->cpu, reg_num, &kind, ctx->anal->bits);
			break;
		}
		case DW_OP_breg0:
		case DW_OP_breg1:
		case DW_OP_breg2:
		case DW_OP_breg3:
		case DW_OP_breg4:
		case DW_OP_breg5:
		case DW_OP_breg6:
		case DW_OP_breg7:
		case DW_OP_breg8:
		case DW_OP_breg9:
		case DW_OP_breg10:
		case DW_OP_breg11:
		case DW_OP_breg12:
		case DW_OP_breg13:
		case DW_OP_breg14:
		case DW_OP_breg15:
		case DW_OP_breg16:
		case DW_OP_breg17:
		case DW_OP_breg18:
		case DW_OP_breg19:
		case DW_OP_breg20:
		case DW_OP_breg21:
		case DW_OP_breg22:
		case DW_OP_breg23:
		case DW_OP_breg24:
		case DW_OP_breg25:
		case DW_OP_breg26:
		case DW_OP_breg27:
		case DW_OP_breg28:
		case DW_OP_breg29:
		case DW_OP_breg30:
		case DW_OP_breg31: {
			if (i == block.length - 1) {
				return NULL;
			}
			reg_num = block.data[i] - DW_OP_breg0;  
			const ut8 *buffer = &block.data[++i];
			offset = r_sleb128 (&buffer, &block.data[block.length]);
			i += buffer - &block.data[0];
			reg_name = get_dwarf_reg_name (ctx->anal->cpu, reg_num, &kind, ctx->anal->bits);
			break;
		}
		case DW_OP_bregx: {
			if (i == block.length - 1) {
				return NULL;
			}
			const ut8 *buffer = &block.data[++i];
			const ut8 *buf_end = &block.data[block.length];
			buffer = r_uleb128 (buffer, buf_end - buffer, &reg_num, NULL);
			if (buffer == buf_end) {
				return NULL;
			}
			offset = r_sleb128 (&buffer, buf_end);
			reg_name = get_dwarf_reg_name (ctx->anal->cpu, reg_num, &kind, ctx->anal->bits);
			break;
		}
		case DW_OP_addr: {
			const int addr_size = ctx->anal->bits / 8;
			const ut8 *dump = &block.data[++i];
			if (block.length - i < addr_size) {
				return NULL;
			}
			switch (addr_size) {
			case 1:
				address = r_read_ble8 (dump);
				break;
			case 2:
				address = r_read_ble16 (dump, ctx->anal->big_endian);
				break;
			case 4:
				address = r_read_ble32 (dump, ctx->anal->big_endian);
				break;
			case 8:
				address = r_read_ble64 (dump, ctx->anal->big_endian);
				break;
			default:
				r_warn_if_reached ();  
				return NULL;
			}
			kind = LOCATION_GLOBAL;  
			break;
		}
		case DW_OP_call_frame_cfa: {
			kind = LOCATION_BP;
			offset += 16;
			break;
		}
		default:
			break;
		}
	}
	if (kind == LOCATION_UNKNOWN) {
		return NULL;
	}
	VariableLocation *location = R_NEW0 (VariableLocation);
	if (location) {
		location->reg_name = reg_name;
		location->reg_num = reg_num;
		location->kind = kind;
		location->offset = offset;
		location->address = address;
	}
	return location;
}