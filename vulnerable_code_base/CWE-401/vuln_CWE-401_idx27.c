LPSTR tr_esc_str(LPCSTR arg, bool format)
{
	LPSTR tmp = NULL;
	size_t cs = 0, x, ds, len;
	size_t s;
	if (NULL == arg)
		return NULL;
	s = strlen(arg);
	while ((s > 0) && isspace(arg[s - 1]))
		s--;
	ds = s + 1;
	if (s)
		tmp = (LPSTR)realloc(tmp, ds * sizeof(CHAR));
	if (NULL == tmp)
	{
		fprintf(stderr,  "Could not allocate string buffer.\n");
		exit(-2);
	}
	memset(tmp, 0, ds * sizeof(CHAR));
	for (x = 0; x < s; x++)
	{
		switch (arg[x])
		{
			case '<':
				len = format ? 13 : 4;
				ds += len - 1;
				tmp = (LPSTR)realloc(tmp, ds * sizeof(CHAR));
				if (NULL == tmp)
				{
					fprintf(stderr,  "Could not reallocate string buffer.\n");
					exit(-3);
				}
				if (format)
					strncpy(&tmp[cs], "<replaceable>", len);
				else
					strncpy(&tmp[cs], "&lt;", len);
				cs += len;
				break;
			case '>':
				len = format ? 14 : 4;
				ds += len - 1;
				tmp = (LPSTR)realloc(tmp, ds * sizeof(CHAR));
				if (NULL == tmp)
				{
					fprintf(stderr,  "Could not reallocate string buffer.\n");
					exit(-4);
				}
				if (format)
					strncpy(&tmp[cs], "</replaceable>", len);
				else
					strncpy(&tmp[cs], "&lt;", len);
				cs += len;
				break;
			case '\'':
				ds += 5;
				tmp = (LPSTR)realloc(tmp, ds * sizeof(CHAR));
				if (NULL == tmp)
				{
					fprintf(stderr,  "Could not reallocate string buffer.\n");
					exit(-5);
				}
				tmp[cs++] = '&';
				tmp[cs++] = 'a';
				tmp[cs++] = 'p';
				tmp[cs++] = 'o';
				tmp[cs++] = 's';
				tmp[cs++] = ';';
				break;
			case '"':
				ds += 5;
				tmp = (LPSTR)realloc(tmp, ds * sizeof(CHAR));
				if (NULL == tmp)
				{
					fprintf(stderr,  "Could not reallocate string buffer.\n");
					exit(-6);
				}
				tmp[cs++] = '&';
				tmp[cs++] = 'q';
				tmp[cs++] = 'u';
				tmp[cs++] = 'o';
				tmp[cs++] = 't';
				tmp[cs++] = ';';
				break;
			case '&':
				ds += 4;
				tmp = (LPSTR)realloc(tmp, ds * sizeof(CHAR));
				if (NULL == tmp)
				{
					fprintf(stderr,  "Could not reallocate string buffer.\n");
					exit(-7);
				}
				tmp[cs++] = '&';
				tmp[cs++] = 'a';
				tmp[cs++] = 'm';
				tmp[cs++] = 'p';
				tmp[cs++] = ';';
				break;
			default:
				tmp[cs++] = arg[x];
				break;
		}
		tmp[ds - 1] = '\0';
	}
	return tmp;
}