void _luac_build_info(LuaProto *proto, LuacBinInfo *info) {
	char *section_name;
	char *symbol_name;
	char *proto_name;
	RzListIter *iter;
	ut64 current_offset;
	ut64 current_size;
	int i = 0;  
	if (proto->name_size == 0 || proto->proto_name == NULL) {
		proto_name = rz_str_newf("fcn.%08llx", proto->offset);
	} else {
		proto_name = rz_str_new((char *)proto->proto_name);
	}
	current_offset = proto->offset;
	current_size = proto->size;
	section_name = rz_str_newf("%s.header", proto_name);
	luac_add_section(info->section_list, section_name, current_offset, current_size, false);
	RZ_FREE(section_name);
	current_offset = proto->code_offset;
	current_size = proto->code_size;
	section_name = rz_str_newf("%s.code", proto_name);
	luac_add_section(info->section_list, section_name, current_offset, current_size, true);
	RZ_FREE(section_name);
	current_offset = proto->const_offset;
	current_size = proto->const_size;
	section_name = rz_str_newf("%s.const", proto_name);
	luac_add_section(info->section_list, section_name, current_offset, current_size, false);
	RZ_FREE(section_name);
	current_offset = proto->upvalue_offset;
	current_size = proto->upvalue_size;
	section_name = rz_str_newf("%s.upvalues", proto_name);
	luac_add_section(info->section_list, section_name, current_offset, current_size, false);
	RZ_FREE(section_name);
	current_offset = proto->inner_proto_offset;
	current_size = proto->inner_proto_size;
	section_name = rz_str_newf("%s.protos", proto_name);
	luac_add_section(info->section_list, section_name, current_offset, current_size, false);
	RZ_FREE(section_name);
	current_offset = proto->debug_offset;
	current_size = proto->debug_size;
	section_name = rz_str_newf("%s.debug", proto_name);
	luac_add_section(info->section_list, section_name, current_offset, current_size, false);
	RZ_FREE(section_name);
	LuaLocalVarEntry *local_var_entry;
	rz_list_foreach (proto->local_var_info_entries, iter, local_var_entry) {
		luac_add_string(
			info->string_list,
			(char *)local_var_entry->varname,
			local_var_entry->offset,
			local_var_entry->varname_len);
	}
	char **upvalue_names;
	int real_upvalue_cnt;
	LuaDbgUpvalueEntry *debug_upv_entry;
	real_upvalue_cnt = rz_list_length(proto->upvalue_entries);
	upvalue_names = RZ_NEWS0(char *, real_upvalue_cnt);
	if (!upvalue_names) {
		return;
	}
	rz_list_foreach (proto->dbg_upvalue_entries, iter, debug_upv_entry) {
		upvalue_names[i] = (char *)debug_upv_entry->upvalue_name;
		luac_add_string(
			info->string_list,
			upvalue_names[i],
			debug_upv_entry->offset,
			debug_upv_entry->name_len);
	}
	LuaConstEntry *const_entry;
	rz_list_foreach (proto->const_entries, iter, const_entry) {
		symbol_name = get_constant_symbol_name(proto_name, const_entry);
		luac_add_symbol(
			info->symbol_list,
			symbol_name,
			const_entry->offset,
			const_entry->data_len,
			get_tag_string(const_entry->tag));
		if (const_entry->tag == LUA_VLNGSTR || const_entry->tag == LUA_VSHRSTR) {
			luac_add_string(
				info->string_list,
				(char *)const_entry->data,
				const_entry->offset,
				const_entry->data_len);
		}
		RZ_FREE(symbol_name);
	}
	LuaUpvalueEntry *upvalue_entry;
	i = 0;
	rz_list_foreach (proto->upvalue_entries, iter, upvalue_entry) {
		symbol_name = get_upvalue_symbol_name(proto_name, upvalue_entry, upvalue_names[i++]);
		luac_add_symbol(
			info->symbol_list,
			symbol_name,
			upvalue_entry->offset,
			3,
			"UPVALUE");
		RZ_FREE(symbol_name);
	}
	LuaProto *sub_proto;
	rz_list_foreach (proto->proto_entries, iter, sub_proto) {
		_luac_build_info(sub_proto, info);
	}
	RZ_FREE(proto_name);
}