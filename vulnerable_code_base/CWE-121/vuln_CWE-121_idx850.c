static char *gdb_to_rz_profile(const char *gdb) {
	rz_return_val_if_fail(gdb, NULL);
	RzStrBuf *sb = rz_strbuf_new("");
	if (!sb) {
		return NULL;
	}
	char *ptr1, *gptr, *gptr1;
	char name[16], groups[128], type[16];
	const int all = 1, gpr = 2, save = 4, restore = 8, float_ = 16,
		  sse = 32, vector = 64, system = 128, mmx = 256;
	int number, rel, offset, size, type_bits, ret;
	const char *ptr = rz_str_trim_head_ro(gdb);
	if (rz_str_startswith(ptr, "Name")) {
		if (!(ptr = strchr(ptr, '\n'))) {
			rz_strbuf_free(sb);
			return NULL;
		}
		ptr++;
	}
	for (;;) {
		while (isspace((ut8)*ptr)) {
			ptr++;
		}
		if (!*ptr) {
			break;
		}
		if ((ptr1 = strchr(ptr, '\n'))) {
			*ptr1 = '\0';
		} else {
			eprintf("Could not parse line: %s (missing \\n)\n", ptr);
			rz_strbuf_free(sb);
			return false;
		}
		ret = sscanf(ptr, " %s %d %d %d %d %s %s", name, &number, &rel,
			&offset, &size, type, groups);
		if (ret < 6) {
			if (*ptr != '*') {
				eprintf("Could not parse line: %s\n", ptr);
				rz_strbuf_free(sb);
				return NULL;
			}
			ptr = ptr1 + 1;
			continue;
		}
		if (rz_str_startswith(name, "''")) {
			if (!ptr1) {
				break;
			}
			ptr = ptr1 + 1;
			continue;
		}
		if (size == 0) {
			if (!ptr1) {
				break;
			}
			ptr = ptr1 + 1;
			continue;
		}
		type_bits = 0;
		if (ret >= 7) {
			gptr = groups;
			while (1) {
				if ((gptr1 = strchr(gptr, ','))) {
					*gptr1 = '\0';
				}
				if (rz_str_startswith(gptr, "general")) {
					type_bits |= gpr;
				} else if (rz_str_startswith(gptr, "all")) {
					type_bits |= all;
				} else if (rz_str_startswith(gptr, "save")) {
					type_bits |= save;
				} else if (rz_str_startswith(gptr, "restore")) {
					type_bits |= restore;
				} else if (rz_str_startswith(gptr, "float")) {
					type_bits |= float_;
				} else if (rz_str_startswith(gptr, "sse")) {
					type_bits |= sse;
				} else if (rz_str_startswith(gptr, "mmx")) {
					type_bits |= mmx;
				} else if (rz_str_startswith(gptr, "vector")) {
					type_bits |= vector;
				} else if (rz_str_startswith(gptr, "system")) {
					type_bits |= system;
				}
				if (!gptr1) {
					break;
				}
				gptr = gptr1 + 1;
			}
		}
		if (!*type) {
			if (!ptr1) {
				break;
			}
			ptr = ptr1 + 1;
			continue;
		}
		if (!(type_bits & sse) && !(type_bits & float_)) {
			type_bits |= gpr;
		}
		rz_strbuf_appendf(sb, "%s\t%s\t.%d\t%d\t0\n",
			((type_bits & mmx) || (type_bits & float_) || (type_bits & sse)) ? "fpu" : "gpr",
			name, size * 8, offset);
		if (!ptr1) {
			break;
		}
		ptr = ptr1 + 1;
		continue;
	}
	return rz_strbuf_drain(sb);
}