parse(struct magic_set *ms, struct magic_entry *me, const char *line,
    size_t lineno, int action)
{
#ifdef ENABLE_CONDITIONALS
	static uint32_t last_cont_level = 0;
#endif
	size_t i;
	struct magic *m;
	const char *l = line;
	char *t;
	int op;
	uint32_t cont_level;
	int32_t diff;
	cont_level = 0;
	while (*l == '>') {
		++l;		 
		cont_level++; 
	}
#ifdef ENABLE_CONDITIONALS
	if (cont_level == 0 || cont_level > last_cont_level)
		if (file_check_mem(ms, cont_level) == -1)
			return -1;
	last_cont_level = cont_level;
#endif
	if (cont_level != 0) {
		if (me->mp == NULL) {
			file_magerror(ms, "No current entry for continuation");
			return -1;
		}
		if (me->cont_count == 0) {
			file_magerror(ms, "Continuations present with 0 count");
			return -1;
		}
		m = &me->mp[me->cont_count - 1];
		diff = (int32_t)cont_level - (int32_t)m->cont_level;
		if (diff > 1)
			file_magwarn(ms, "New continuation level %u is more "
			    "than one larger than current level %u", cont_level,
			    m->cont_level);
		if (me->cont_count == me->max_count) {
			struct magic *nm;
			size_t cnt = me->max_count + ALLOC_CHUNK;
			if ((nm = CAST(struct magic *, realloc(me->mp,
			    sizeof(*nm) * cnt))) == NULL) {
				file_oomem(ms, sizeof(*nm) * cnt);
				return -1;
			}
			me->mp = m = nm;
			me->max_count = CAST(uint32_t, cnt);
		}
		m = &me->mp[me->cont_count++];
		(void)memset(m, 0, sizeof(*m));
		m->cont_level = cont_level;
	} else {
		static const size_t len = sizeof(*m) * ALLOC_CHUNK;
		if (me->mp != NULL)
			return 1;
		if ((m = CAST(struct magic *, malloc(len))) == NULL) {
			file_oomem(ms, len);
			return -1;
		}
		me->mp = m;
		me->max_count = ALLOC_CHUNK;
		(void)memset(m, 0, sizeof(*m));
		m->factor_op = FILE_FACTOR_OP_NONE;
		m->cont_level = 0;
		me->cont_count = 1;
	}
	m->lineno = CAST(uint32_t, lineno);
	if (*l == '&') {   
                ++l;             
                m->flag |= OFFADD;
        }
	if (*l == '(') {
		++l;		 
		m->flag |= INDIR;
		if (m->flag & OFFADD)
			m->flag = (m->flag & ~OFFADD) | INDIROFFADD;
		if (*l == '&') {   
			++l;             
			m->flag |= OFFADD;
		}
	}
	if (m->cont_level == 0 && (m->flag & (OFFADD | INDIROFFADD)))
		if (ms->flags & MAGIC_CHECK)
			file_magwarn(ms, "relative offset at level 0");
	m->offset = (uint32_t)strtoul(l, &t, 0);
        if (l == t)
		if (ms->flags & MAGIC_CHECK)
			file_magwarn(ms, "offset `%s' invalid", l);
        l = t;
	if (m->flag & INDIR) {
		m->in_type = FILE_LONG;
		m->in_offset = 0;
		if (*l == '.') {
			l++;
			switch (*l) {
			case 'l':
				m->in_type = FILE_LELONG;
				break;
			case 'L':
				m->in_type = FILE_BELONG;
				break;
			case 'm':
				m->in_type = FILE_MELONG;
				break;
			case 'h':
			case 's':
				m->in_type = FILE_LESHORT;
				break;
			case 'H':
			case 'S':
				m->in_type = FILE_BESHORT;
				break;
			case 'c':
			case 'b':
			case 'C':
			case 'B':
				m->in_type = FILE_BYTE;
				break;
			case 'e':
			case 'f':
			case 'g':
				m->in_type = FILE_LEDOUBLE;
				break;
			case 'E':
			case 'F':
			case 'G':
				m->in_type = FILE_BEDOUBLE;
				break;
			case 'i':
				m->in_type = FILE_LEID3;
				break;
			case 'I':
				m->in_type = FILE_BEID3;
				break;
			default:
				if (ms->flags & MAGIC_CHECK)
					file_magwarn(ms,
					    "indirect offset type `%c' invalid",
					    *l);
				break;
			}
			l++;
		}
		m->in_op = 0;
		if (*l == '~') {
			m->in_op |= FILE_OPINVERSE;
			l++;
		}
		if ((op = get_op(*l)) != -1) {
			m->in_op |= op;
			l++;
		}
		if (*l == '(') {
			m->in_op |= FILE_OPINDIRECT;
			l++;
		}
		if (isdigit((unsigned char)*l) || *l == '-') {
			m->in_offset = (int32_t)strtol(l, &t, 0);
			if (l == t)
				if (ms->flags & MAGIC_CHECK)
					file_magwarn(ms,
					    "in_offset `%s' invalid", l);
			l = t;
		}
		if (*l++ != ')' || 
		    ((m->in_op & FILE_OPINDIRECT) && *l++ != ')'))
			if (ms->flags & MAGIC_CHECK)
				file_magwarn(ms,
				    "missing ')' in indirect offset");
	}
	EATAB;
#ifdef ENABLE_CONDITIONALS
	m->cond = get_cond(l, &l);
	if (check_cond(ms, m->cond, cont_level) == -1)
		return -1;
	EATAB;
#endif
	if (*l == 'u') {
		m->type = get_type(type_tbl, l + 1, &l);
		if (m->type == FILE_INVALID) {
			m->type = get_standard_integer_type(l, &l);
		}
		if (m->type != FILE_INVALID)
			m->flag |= UNSIGNED;
	} else {
		m->type = get_type(type_tbl, l, &l);
		if (m->type == FILE_INVALID) {
			if (*l == 'd')
				m->type = get_standard_integer_type(l, &l);
			else if (*l == 's' && !isalpha((unsigned char)l[1])) {
				m->type = FILE_STRING;
				++l;
			}
		}
	}
	if (m->type == FILE_INVALID) {
		m->type = get_type(special_tbl, l, &l);
	}
	if (m->type == FILE_INVALID) {
		if (ms->flags & MAGIC_CHECK)
			file_magwarn(ms, "type `%s' invalid", l);
		return -1;
	}
	m->mask_op = 0;
	if (*l == '~') {
		if (!IS_STRING(m->type))
			m->mask_op |= FILE_OPINVERSE;
		else if (ms->flags & MAGIC_CHECK)
			file_magwarn(ms, "'~' invalid for string types");
		++l;
	}
	m->str_range = 0;
	m->str_flags = m->type == FILE_PSTRING ? PSTRING_1_LE : 0;
	if ((op = get_op(*l)) != -1) {
		if (!IS_STRING(m->type)) {
			uint64_t val;
			++l;
			m->mask_op |= op;
			val = (uint64_t)strtoull(l, &t, 0);
			l = t;
			m->num_mask = file_signextend(ms, m, val);
			eatsize(&l);
		}
		else if (op == FILE_OPDIVIDE) {
			int have_range = 0;
			while (!isspace((unsigned char)*++l)) {
				switch (*l) {
				case '0':  case '1':  case '2':
				case '3':  case '4':  case '5':
				case '6':  case '7':  case '8':
				case '9':
					if (have_range &&
					    (ms->flags & MAGIC_CHECK))
						file_magwarn(ms,
						    "multiple ranges");
					have_range = 1;
					m->str_range = CAST(uint32_t,
					    strtoul(l, &t, 0));
					if (m->str_range == 0)
						file_magwarn(ms,
						    "zero range");
					l = t - 1;
					break;
				case CHAR_COMPACT_WHITESPACE:
					m->str_flags |=
					    STRING_COMPACT_WHITESPACE;
					break;
				case CHAR_COMPACT_OPTIONAL_WHITESPACE:
					m->str_flags |=
					    STRING_COMPACT_OPTIONAL_WHITESPACE;
					break;
				case CHAR_IGNORE_LOWERCASE:
					m->str_flags |= STRING_IGNORE_LOWERCASE;
					break;
				case CHAR_IGNORE_UPPERCASE:
					m->str_flags |= STRING_IGNORE_UPPERCASE;
					break;
				case CHAR_REGEX_OFFSET_START:
					m->str_flags |= REGEX_OFFSET_START;
					break;
				case CHAR_BINTEST:
					m->str_flags |= STRING_BINTEST;
					break;
				case CHAR_TEXTTEST:
					m->str_flags |= STRING_TEXTTEST;
					break;
				case CHAR_TRIM:
					m->str_flags |= STRING_TRIM;
					break;
				case CHAR_PSTRING_1_LE:
					if (m->type != FILE_PSTRING)
						goto bad;
					m->str_flags = (m->str_flags & ~PSTRING_LEN) | PSTRING_1_LE;
					break;
				case CHAR_PSTRING_2_BE:
					if (m->type != FILE_PSTRING)
						goto bad;
					m->str_flags = (m->str_flags & ~PSTRING_LEN) | PSTRING_2_BE;
					break;
				case CHAR_PSTRING_2_LE:
					if (m->type != FILE_PSTRING)
						goto bad;
					m->str_flags = (m->str_flags & ~PSTRING_LEN) | PSTRING_2_LE;
					break;
				case CHAR_PSTRING_4_BE:
					if (m->type != FILE_PSTRING)
						goto bad;
					m->str_flags = (m->str_flags & ~PSTRING_LEN) | PSTRING_4_BE;
					break;
				case CHAR_PSTRING_4_LE:
					if (m->type != FILE_PSTRING)
						goto bad;
					m->str_flags = (m->str_flags & ~PSTRING_LEN) | PSTRING_4_LE;
					break;
				case CHAR_PSTRING_LENGTH_INCLUDES_ITSELF:
					if (m->type != FILE_PSTRING)
						goto bad;
					m->str_flags |= PSTRING_LENGTH_INCLUDES_ITSELF;
					break;
				default:
				bad:
					if (ms->flags & MAGIC_CHECK)
						file_magwarn(ms,
						    "string extension `%c' "
						    "invalid", *l);
					return -1;
				}
				if (l[1] == '/' &&
				    !isspace((unsigned char)l[2]))
					l++;
			}
			if (string_modifier_check(ms, m) == -1)
				return -1;
		}
		else {
			if (ms->flags & MAGIC_CHECK)
				file_magwarn(ms, "invalid string op: %c", *t);
			return -1;
		}
	}
	EATAB;
	switch (*l) {
	case '>':
	case '<':
  		m->reln = *l;
  		++l;
		if (*l == '=') {
			if (ms->flags & MAGIC_CHECK) {
				file_magwarn(ms, "%c= not supported",
				    m->reln);
				return -1;
			}
		   ++l;
		}
		break;
	case '&':
	case '^':
	case '=':
  		m->reln = *l;
  		++l;
		if (*l == '=') {
		   ++l;
		}
		break;
	case '!':
		m->reln = *l;
		++l;
		break;
	default:
  		m->reln = '=';	 
		if (*l == 'x' && ((isascii((unsigned char)l[1]) && 
		    isspace((unsigned char)l[1])) || !l[1])) {
			m->reln = *l;
			++l;
		}
		break;
	}
	if (m->reln != 'x' && getvalue(ms, m, &l, action))
		return -1;
	EATAB;
	if (l[0] == '\b') {
		++l;
		m->flag |= NOSPACE;
	} else if ((l[0] == '\\') && (l[1] == 'b')) {
		++l;
		++l;
		m->flag |= NOSPACE;
	}
	for (i = 0; (m->desc[i++] = *l++) != '\0' && i < sizeof(m->desc); )
		continue;
	if (i == sizeof(m->desc)) {
		m->desc[sizeof(m->desc) - 1] = '\0';
		if (ms->flags & MAGIC_CHECK)
			file_magwarn(ms, "description `%s' truncated", m->desc);
	}
        if (ms->flags & MAGIC_CHECK) {
		if (check_format(ms, m) == -1)
			return -1;
	}
#ifndef COMPILE_ONLY
	if (action == FILE_CHECK) {
		file_mdump(m);
	}
#endif
	m->mimetype[0] = '\0';		 
	return 0;
}