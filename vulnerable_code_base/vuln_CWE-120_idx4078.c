static RzList   *__io_maps(RzDebug *dbg) {
	RzList *list = rz_list_new();
	char *str = dbg->iob.system(dbg->iob.io, "dm");
	if (!str) {
		rz_list_free(list);
		return NULL;
	}
	char *ostr = str;
	ut64 map_start, map_end;
	char perm[32];
	char name[512];
	for (;;) {
		char *nl = strchr(str, '\n');
		if (nl) {
			*nl = 0;
			*name = 0;
			*perm = 0;
			map_start = map_end = 0LL;
			if (!strncmp(str, "sys ", 4)) {
				char *sp = strchr(str + 4, ' ');
				if (sp) {
					str = sp + 1;
				} else {
					str += 4;
				}
			}
			char *_s_ = strstr(str, " s ");
			if (_s_) {
				memmove(_s_, _s_ + 2, strlen(_s_));
			}
			_s_ = strstr(str, " ? ");
			if (_s_) {
				memmove(_s_, _s_ + 2, strlen(_s_));
			}
			sscanf(str, "0x%" PFMT64x " - 0x%" PFMT64x " %s %s",
				&map_start, &map_end, perm, name);
			if (map_end != 0LL) {
				RzDebugMap *map = rz_debug_map_new(name, map_start, map_end, rz_str_rwx(perm), 0);
				rz_list_append(list, map);
			}
			str = nl + 1;
		} else {
			break;
		}
	}
	free(ostr);
	rz_cons_reset();
	return list;
}