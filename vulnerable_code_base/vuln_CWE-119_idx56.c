getword(f, word, newlinep, filename)
    FILE *f;
    char *word;
    int *newlinep;
    char *filename;
{
    int c, len, escape;
    int quoted, comment;
    int value, digit, got, n;
#define isoctal(c) ((c) >= '0' && (c) < '8')
    *newlinep = 0;
    len = 0;
    escape = 0;
    comment = 0;
    quoted = 0;
    for (;;) {
	c = getc(f);
	if (c == EOF)
	    break;
	if (c == '\n') {
	    if (!escape) {
		*newlinep = 1;
		comment = 0;
	    } else
		escape = 0;
	    continue;
	}
	if (comment)
	    continue;
	if (escape)
	    break;
	if (c == '\\') {
	    escape = 1;
	    continue;
	}
	if (c == '#') {
	    comment = 1;
	    continue;
	}
	if (!isspace(c))
	    break;
    }
    while (c != EOF) {
	if (escape) {
	    escape = 0;
	    if (c == '\n') {
	        c = getc(f);
		continue;
	    }
	    got = 0;
	    switch (c) {
	    case 'a':
		value = '\a';
		break;
	    case 'b':
		value = '\b';
		break;
	    case 'f':
		value = '\f';
		break;
	    case 'n':
		value = '\n';
		break;
	    case 'r':
		value = '\r';
		break;
	    case 's':
		value = ' ';
		break;
	    case 't':
		value = '\t';
		break;
	    default:
		if (isoctal(c)) {
		    value = 0;
		    for (n = 0; n < 3 && isoctal(c); ++n) {
			value = (value << 3) + (c & 07);
			c = getc(f);
		    }
		    got = 1;
		    break;
		}
		if (c == 'x') {
		    value = 0;
		    c = getc(f);
		    for (n = 0; n < 2 && isxdigit(c); ++n) {
			digit = toupper(c) - '0';
			if (digit > 10)
			    digit += '0' + 10 - 'A';
			value = (value << 4) + digit;
			c = getc (f);
		    }
		    got = 1;
		    break;
		}
		value = c;
		break;
	    }
	    if (len < MAXWORDLEN-1)
		word[len] = value;
	    ++len;
	    if (!got)
		c = getc(f);
	    continue;
	}
	if (c == '\\') {
	    escape = 1;
	    c = getc(f);
	    continue;
	}
	if (quoted) {
	    if (c == quoted) {
		quoted = 0;
		c = getc(f);
		continue;
	    }
	} else if (c == '"' || c == '\'') {
	    quoted = c;
	    c = getc(f);
	    continue;
	} else if (isspace(c) || c == '#') {
	    ungetc (c, f);
	    break;
	}
	if (len < MAXWORDLEN-1)
	    word[len] = c;
	++len;
	c = getc(f);
    }
    if (c == EOF) {
	if (ferror(f)) {
	    if (errno == 0)
		errno = EIO;
	    option_error("Error reading %s: %m", filename);
	    die(1);
	}
	if (len == 0)
	    return 0;
	if (quoted)
	    option_error("warning: quoted word runs to end of file (%.20s...)",
			 filename, word);
    }
    if (len >= MAXWORDLEN) {
	option_error("warning: word in file %s too long (%.20s...)",
		     filename, word);
	len = MAXWORDLEN - 1;
    }
    word[len] = 0;
    return 1;
#undef isoctal
}