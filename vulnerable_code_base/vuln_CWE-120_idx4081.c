static RzList   *rz_debug_gdb_map_get(RzDebug *dbg) {  
	RzDebugGdbCtx *ctx = dbg->plugin_data;
	check_connection(dbg);
	if (!ctx->desc || ctx->desc->pid <= 0) {
		return NULL;
	}
	RzList *retlist = NULL;
	if (ctx->desc->get_baddr) {
		ctx->desc->get_baddr = false;
		ut64 baddr;
		if ((baddr = gdbr_get_baddr(ctx->desc)) != UINT64_MAX) {
			if (!(retlist = rz_list_new())) {
				return NULL;
			}
			RzDebugMap *map;
			if (!(map = rz_debug_map_new("", baddr, baddr, RZ_PERM_RX, 0))) {
				rz_list_free(retlist);
				return NULL;
			}
			rz_list_append(retlist, map);
			return retlist;
		}
	}
	char path[128];
	ut8 *buf;
	int ret;
	ut64 buflen = 16384;
	snprintf(path, sizeof(path) - 1, "/proc/%d/maps", ctx->desc->pid);
#ifdef _MSC_VER
#define GDB_FILE_OPEN_MODE (_S_IREAD | _S_IWRITE)
#else
#define GDB_FILE_OPEN_MODE (S_IRUSR | S_IWUSR | S_IXUSR)
#endif
	if (gdbr_open_file(ctx->desc, path, O_RDONLY, GDB_FILE_OPEN_MODE) < 0) {
		return NULL;
	}
	if (!(buf = malloc(buflen))) {
		gdbr_close_file(ctx->desc);
		return NULL;
	}
	if ((ret = gdbr_read_file(ctx->desc, buf, buflen - 1)) <= 0) {
		gdbr_close_file(ctx->desc);
		free(buf);
		return NULL;
	}
	buf[ret] = '\0';
	int unk = 0, perm, i;
	char *ptr, *pos_1;
	size_t line_len;
	char name[1024], region1[100], region2[100], perms[5];
	RzDebugMap *map = NULL;
	region1[0] = region2[0] = '0';
	region1[1] = region2[1] = 'x';
	if (!(ptr = strtok((char *)buf, "\n"))) {
		gdbr_close_file(ctx->desc);
		free(buf);
		return NULL;
	}
	if (!(retlist = rz_list_new())) {
		gdbr_close_file(ctx->desc);
		free(buf);
		return NULL;
	}
	while (ptr) {
		ut64 map_start, map_end, offset;
		bool map_is_shared = false;
		line_len = strlen(ptr);
		if (line_len == 0) {
			break;
		}
		ret = sscanf(ptr, "%s %s %" PFMT64x " %*s %*s %[^\n]", &region1[2],
			perms, &offset, name);
		if (ret == 3) {
			name[0] = '\0';
		} else if (ret != 4) {
			eprintf("%s: Unable to parse \"%s\"\nContent:\n%s\n",
				__func__, path, buf);
			gdbr_close_file(ctx->desc);
			free(buf);
			rz_list_free(retlist);
			return NULL;
		}
		if (!(pos_1 = strchr(&region1[2], '-'))) {
			ptr = strtok(NULL, "\n");
			continue;
		}
		strncpy(&region2[2], pos_1 + 1, sizeof(region2) - 2 - 1);
		if (!*name) {
			snprintf(name, sizeof(name), "unk%d", unk++);
		}
		perm = 0;
		for (i = 0; i < 5 && perms[i]; i++) {
			switch (perms[i]) {
			case 'r': perm |= RZ_PERM_R; break;
			case 'w': perm |= RZ_PERM_W; break;
			case 'x': perm |= RZ_PERM_X; break;
			case 'p': map_is_shared = false; break;
			case 's': map_is_shared = true; break;
			}
		}
		map_start = rz_num_get(NULL, region1);
		map_end = rz_num_get(NULL, region2);
		if (map_start == map_end || map_end == 0) {
			eprintf("%s: ignoring invalid map size: %s - %s\n",
				__func__, region1, region2);
			ptr = strtok(NULL, "\n");
			continue;
		}
		if (!(map = rz_debug_map_new(name, map_start, map_end, perm, 0))) {
			break;
		}
		map->offset = offset;
		map->shared = map_is_shared;
		map->file = strdup(name);
		rz_list_append(retlist, map);
		ptr = strtok(NULL, "\n");
	}
	gdbr_close_file(ctx->desc);
	free(buf);
	return retlist;
}