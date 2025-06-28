static int cmd_meta_comment(RCore *core, const char *input) {
	ut64 addr = core->offset;
	switch (input[1]) {
	case '?':
		r_core_cmd_help (core, help_msg_CC);
		break;
	case ',':  
		r_meta_print_list_all (core->anal, R_META_TYPE_COMMENT, ',', input + 2);
		break;
	case 'F':  
		if (input[2]=='?') {
			eprintf ("Usage: CCF [file]\n");
		} else if (input[2] == ' ') {
			const char *fn = input + 2;
			const char *comment = r_meta_get_string (core->anal, R_META_TYPE_COMMENT, addr);
			fn = r_str_trim_head_ro (fn);
			if (comment && *comment) {
				char *nc = r_str_newf ("%s ,(%s)", comment, fn);
				r_meta_set_string (core->anal, R_META_TYPE_COMMENT, addr, nc);
				free (nc);
			} else {
				char *newcomment = r_str_newf (",(%s)", fn);
				r_meta_set_string (core->anal, R_META_TYPE_COMMENT, addr, newcomment);
				free (newcomment);
			}
		} else {
			const char *comment = r_meta_get_string (core->anal, R_META_TYPE_COMMENT, addr);
			if (comment && *comment) {
				char *cmtfile = r_str_between (comment, ",(", ")");
				if (cmtfile && *cmtfile) {
					char *cwd = getcommapath (core);
					r_cons_printf ("%s"R_SYS_DIR"%s\n", cwd, cmtfile);
					free (cwd);
				}
				free (cmtfile);
			}
		}
		break;
	case '.':
		  {
			  ut64 at = input[2]? r_num_math (core->num, input + 2): addr;
			  const char *comment = r_meta_get_string (core->anal, R_META_TYPE_COMMENT, at);
			  if (comment) {
				  r_cons_println (comment);
			  }
		  }
		break;
	case 0:  
		r_meta_print_list_all (core->anal, R_META_TYPE_COMMENT, 0, NULL);
		break;
	case 'f':  
		switch (input[2]) {
		case '-':  
			{
				ut64 arg = r_num_math (core->num, input + 2);
				if (!arg) {
					arg = core->offset;
				}
				RAnalFunction *fcn = r_anal_get_fcn_in (core->anal, arg, 0);
				if (fcn) {
					RAnalBlock *bb;
					RListIter *iter;
					r_list_foreach (fcn->bbs, iter, bb) {
						int i;
						for (i = 0; i < bb->size; i++) {
							ut64 addr = bb->addr + i;
							r_meta_del (core->anal, R_META_TYPE_COMMENT, addr, 1);
						}
					}
				}
			}
			break;
		case ',':  
			r_meta_print_list_in_function (core->anal, R_META_TYPE_COMMENT, ',', core->offset, input + 3);
			break;
		case 'j':  
			r_meta_print_list_in_function (core->anal, R_META_TYPE_COMMENT, 'j', core->offset, NULL);
			break;
		case '*':  
			r_meta_print_list_in_function (core->anal, R_META_TYPE_COMMENT, 1, core->offset, NULL);
			break;
		default:
			r_meta_print_list_in_function (core->anal, R_META_TYPE_COMMENT, 0, core->offset, NULL);
			break;
		}
		break;
	case 'j':  
		r_meta_print_list_all (core->anal, R_META_TYPE_COMMENT, 'j', input + 2);
		break;
	case '!':
		{
			char *out;
			const char *comment = r_meta_get_string (core->anal, R_META_TYPE_COMMENT, addr);
			out = r_core_editor (core, NULL, comment);
			if (out) {
				r_core_cmdf (core, "CC-@0x%08"PFMT64x, addr);
				r_meta_set_string (core->anal,
						R_META_TYPE_COMMENT, addr, out);
				free (out);
			}
		}
		break;
	case '+':
	case ' ':
		{
		const char *newcomment = r_str_trim_head_ro (input + 2);
		const char *comment = r_meta_get_string (core->anal, R_META_TYPE_COMMENT, addr);
		char *text;
		char *nc = strdup (newcomment);
		r_str_unescape (nc);
		if (comment) {
			text = malloc (strlen (comment) + strlen (newcomment) + 2);
			if (text) {
				strcpy (text, comment);
				strcat (text, " ");
				strcat (text, nc);
				r_meta_set_string (core->anal, R_META_TYPE_COMMENT, addr, text);
				free (text);
			} else {
				r_sys_perror ("malloc");
			}
		} else {
			r_meta_set_string (core->anal, R_META_TYPE_COMMENT, addr, nc);
			if (r_config_get_b (core->config, "cmd.undo")) {
				char *a = r_str_newf ("CC-0x%08"PFMT64x, addr);
				char *b = r_str_newf ("CC %s@0x%08"PFMT64x, nc, addr);
				RCoreUndo *uc = r_core_undo_new (core->offset, b, a);
				r_core_undo_push (core, uc);
				free (a);
				free (b);
			}
		}
		free (nc);
		}
		break;
	case '*':  
		r_meta_print_list_all (core->anal, R_META_TYPE_COMMENT, 1, NULL);
		break;
	case '-':  
		if (input[2] == '*') {  
			r_meta_del (core->anal, R_META_TYPE_COMMENT, UT64_MAX, UT64_MAX);
		} else if (input[2]) {  
			ut64 arg = r_num_math (core->num, input + 2);
			r_meta_del (core->anal, R_META_TYPE_COMMENT, arg, 1);
		} else {  
			r_meta_del (core->anal, R_META_TYPE_COMMENT, core->offset, 1);
		}
		break;
	case 'u':  
		{
		char *newcomment;
		const char *arg = input + 2;
		while (*arg && *arg == ' ') arg++;
		if (!strncmp (arg, "base64:", 7)) {
			char *s = (char *)sdb_decode (arg + 7, NULL);
			if (s) {
				newcomment = s;
			} else {
				newcomment = NULL;
			}
		} else {
			newcomment = strdup (arg);
		}
		if (newcomment) {
			const char *comment = r_meta_get_string (core->anal, R_META_TYPE_COMMENT, addr);
			if (!comment || (comment && !strstr (comment, newcomment))) {
				r_meta_set_string (core->anal, R_META_TYPE_COMMENT, addr, newcomment);
			}
			free (newcomment);
		}
		}
		break;
	case 'a':  
		{
		char *s, *p;
		s = strchr (input, ' ');
		if (s) {
			s = strdup (s + 1);
		} else {
			eprintf ("Usage: CCa [address] [comment]\n");
			eprintf ("Usage: CCa-[address]\n");
			return false;
		}
		p = strchr (s, ' ');
		if (p) {
			*p++ = 0;
		}
		ut64 addr;
		if (input[2] == '-') {
			if (input[3]) {
				addr = r_num_math (core->num, input+3);
				r_meta_del (core->anal,
						R_META_TYPE_COMMENT,
						addr, 1);
			} else {
				eprintf ("Usage: CCa-[address]\n");
			}
			free (s);
			return true;
		}
		addr = r_num_math (core->num, s);
		if (p) {
			if (input[2]=='+') {
				const char *comment = r_meta_get_string (core->anal, R_META_TYPE_COMMENT, addr);
				if (comment) {
					char *text = r_str_newf ("%s\n%s", comment, p);
					r_meta_set (core->anal, R_META_TYPE_COMMENT, addr, 1, text);
					free (text);
				} else {
					r_meta_set (core->anal, R_META_TYPE_COMMENT, addr, 1, p);
				}
			} else {
				r_meta_set (core->anal, R_META_TYPE_COMMENT, addr, 1, p);
			}
		} else {
			eprintf ("Usage: CCa [address] [comment]\n");
		}
		free (s);
		return true;
		}
	}
	return true;
}