smb_fdata1(netdissect_options *ndo,
           const u_char *buf, const char *fmt, const u_char *maxbuf,
           int unicodestr)
{
    int reverse = 0;
    const char *attrib_fmt = "READONLY|HIDDEN|SYSTEM|VOLUME|DIR|ARCHIVE|";
    char strbuf[MAX_UNISTR_SIZE+1];
    while (*fmt && buf<maxbuf) {
	switch (*fmt) {
	case 'a':
	    write_bits(ndo, GET_U_1(buf), attrib_fmt);
	    buf++;
	    fmt++;
	    break;
	case 'A':
	    write_bits(ndo, GET_LE_U_2(buf), attrib_fmt);
	    buf += 2;
	    fmt++;
	    break;
	case '{':
	  {
	    char bitfmt[128];
	    char *p;
	    u_int l;
	    p = strchr(++fmt, '}');
	    l = ND_BYTES_BETWEEN(p, fmt);
	    if (l > sizeof(bitfmt) - 1)
		l = sizeof(bitfmt)-1;
	    strncpy(bitfmt, fmt, l);
	    bitfmt[l] = '\0';
	    fmt = p + 1;
	    write_bits(ndo, GET_U_1(buf), bitfmt);
	    buf++;
	    break;
	  }
	case 'P':
	  {
	    int l = atoi(fmt + 1);
	    ND_TCHECK_LEN(buf, l);
	    buf += l;
	    fmt++;
	    while (ND_ASCII_ISDIGIT(*fmt))
		fmt++;
	    break;
	  }
	case 'r':
	    reverse = !reverse;
	    fmt++;
	    break;
	case 'b':
	  {
	    unsigned int x;
	    x = GET_U_1(buf);
	    ND_PRINT("%u (0x%x)", x, x);
	    buf += 1;
	    fmt++;
	    break;
	  }
	case 'd':
	  {
	    int x;
	    x = reverse ? GET_BE_S_2(buf) :
			  GET_LE_S_2(buf);
	    ND_PRINT("%d (0x%x)", x, x);
	    buf += 2;
	    fmt++;
	    break;
	  }
	case 'D':
	  {
	    int x;
	    x = reverse ? GET_BE_S_4(buf) :
			  GET_LE_S_4(buf);
	    ND_PRINT("%d (0x%x)", x, x);
	    buf += 4;
	    fmt++;
	    break;
	  }
	case 'L':
	  {
	    uint64_t x;
	    x = reverse ? GET_BE_U_8(buf) :
			  GET_LE_U_8(buf);
	    ND_PRINT("%" PRIu64 " (0x%" PRIx64 ")", x, x);
	    buf += 8;
	    fmt++;
	    break;
	  }
	case 'u':
	  {
	    unsigned int x;
	    x = reverse ? GET_BE_U_2(buf) :
			  GET_LE_U_2(buf);
	    ND_PRINT("%u (0x%x)", x, x);
	    buf += 2;
	    fmt++;
	    break;
	  }
	case 'U':
	  {
	    unsigned int x;
	    x = reverse ? GET_BE_U_4(buf) :
			  GET_LE_U_4(buf);
	    ND_PRINT("%u (0x%x)", x, x);
	    buf += 4;
	    fmt++;
	    break;
	  }
	case 'M':
	  {
	    uint32_t x1, x2;
	    uint64_t x;
	    ND_TCHECK_8(buf);
	    x1 = reverse ? GET_BE_U_4(buf) :
			   GET_LE_U_4(buf);
	    x2 = reverse ? GET_BE_U_4(buf + 4) :
			   GET_LE_U_4(buf + 4);
	    x = (((uint64_t)x1) << 32) | x2;
	    ND_PRINT("%" PRIu64 " (0x%" PRIx64 ")", x, x);
	    buf += 8;
	    fmt++;
	    break;
	  }
	case 'B':
	  {
	    unsigned int x;
	    x = GET_U_1(buf);
	    ND_PRINT("0x%X", x);
	    buf += 1;
	    fmt++;
	    break;
	  }
	case 'w':
	  {
	    unsigned int x;
	    x = reverse ? GET_BE_U_2(buf) :
			  GET_LE_U_2(buf);
	    ND_PRINT("0x%X", x);
	    buf += 2;
	    fmt++;
	    break;
	  }
	case 'W':
	  {
	    unsigned int x;
	    x = reverse ? GET_BE_U_4(buf) :
			  GET_LE_U_4(buf);
	    ND_PRINT("0x%X", x);
	    buf += 4;
	    fmt++;
	    break;
	  }
	case 'l':
	  {
	    fmt++;
	    switch (*fmt) {
	    case 'b':
		stringlen = GET_U_1(buf);
		stringlen_is_set = 1;
		ND_PRINT("%u", stringlen);
		buf += 1;
		break;
	    case 'd':
	    case 'u':
		stringlen = reverse ? GET_BE_U_2(buf) :
				      GET_LE_U_2(buf);
		stringlen_is_set = 1;
		ND_PRINT("%u", stringlen);
		buf += 2;
		break;
	    case 'D':
	    case 'U':
		stringlen = reverse ? GET_BE_U_4(buf) :
				      GET_LE_U_4(buf);
		stringlen_is_set = 1;
		ND_PRINT("%u", stringlen);
		buf += 4;
		break;
	    }
	    fmt++;
	    break;
	  }
	case 'S':
	case 'R':	 
	  {
	    buf = unistr(ndo, &strbuf, buf, 0, 1, (*fmt == 'R') ? 0 : unicodestr);
	    ND_PRINT("%s", strbuf);
	    if (buf == NULL)
		goto trunc;
	    fmt++;
	    break;
	  }
	case 'Z':
	case 'Y':	 
	  {
	    if (GET_U_1(buf) != 4 && GET_U_1(buf) != 2) {
		ND_PRINT("Error! ASCIIZ buffer of type %u", GET_U_1(buf));
		return maxbuf;	 
	    }
	    buf = unistr(ndo, &strbuf, buf + 1, 0, 1, (*fmt == 'Y') ? 0 : unicodestr);
	    ND_PRINT("%s", strbuf);
	    if (buf == NULL)
		goto trunc;
	    fmt++;
	    break;
	  }
	case 's':
	  {
	    int l = atoi(fmt + 1);
	    ND_TCHECK_LEN(buf, l);
	    ND_PRINT("%-*.*s", l, l, buf);
	    buf += l;
	    fmt++;
	    while (ND_ASCII_ISDIGIT(*fmt))
		fmt++;
	    break;
	  }
	case 'c':
	  {
            if (!stringlen_is_set) {
                ND_PRINT("{stringlen not set}");
                goto trunc;
            }
	    ND_TCHECK_LEN(buf, stringlen);
	    ND_PRINT("%-*.*s", (int)stringlen, (int)stringlen, buf);
	    buf += stringlen;
	    fmt++;
	    while (ND_ASCII_ISDIGIT(*fmt))
		fmt++;
	    break;
	  }
	case 'C':
	  {
            if (!stringlen_is_set) {
                ND_PRINT("{stringlen not set}");
                goto trunc;
            }
	    buf = unistr(ndo, &strbuf, buf, stringlen, 0, unicodestr);
	    ND_PRINT("%s", strbuf);
	    if (buf == NULL)
		goto trunc;
	    fmt++;
	    break;
	  }
	case 'h':
	  {
	    int l = atoi(fmt + 1);
	    ND_TCHECK_LEN(buf, l);
	    while (l--) {
		ND_PRINT("%02x", GET_U_1(buf));
		buf++;
	    }
	    fmt++;
	    while (ND_ASCII_ISDIGIT(*fmt))
		fmt++;
	    break;
	  }
	case 'n':
	  {
	    int t = atoi(fmt+1);
	    char nbuf[255];
	    int name_type;
	    int len;
	    switch (t) {
	    case 1:
		name_type = name_extract(ndo, startbuf, ND_BYTES_BETWEEN(buf, startbuf),
		    maxbuf, nbuf);
		if (name_type < 0)
		    goto trunc;
		len = name_len(ndo, buf, maxbuf);
		if (len < 0)
		    goto trunc;
		buf += len;
		ND_PRINT("%-15.15s NameType=0x%02X (%s)", nbuf, name_type,
		    name_type_str(name_type));
		break;
	    case 2:
		name_type = GET_U_1(buf + 15);
		ND_PRINT("%-15.15s NameType=0x%02X (%s)", buf, name_type,
		    name_type_str(name_type));
		buf += 16;
		break;
	    }
	    fmt++;
	    while (ND_ASCII_ISDIGIT(*fmt))
		fmt++;
	    break;
	  }
	case 'T':
	  {
	    time_t t;
	    struct tm *lt;
	    const char *tstring;
	    char buffer[sizeof("Www Mmm dd hh:mm:ss yyyyy\n")];
	    uint32_t x;
	    switch (atoi(fmt + 1)) {
	    case 1:
		x = GET_LE_U_4(buf);
		if (x == 0 || x == 0xFFFFFFFF)
		    t = 0;
		else
		    t = make_unix_date(ndo, buf);
		buf += 4;
		break;
	    case 2:
		x = GET_LE_U_4(buf);
		if (x == 0 || x == 0xFFFFFFFF)
		    t = 0;
		else
		    t = make_unix_date2(ndo, buf);
		buf += 4;
		break;
	    case 3:
		ND_TCHECK_8(buf);
		t = interpret_long_date(ndo, buf);
		buf += 8;
		break;
	    default:
		t = 0;
		break;
	    }
	    if (t != 0) {
		lt = localtime(&t);
		if (lt != NULL) {
		    strftime(buffer, sizeof(buffer), "%a %b %e %T %Y%n", lt);
		    tstring = buffer;
		}
		else
		    tstring = "(Can't convert time)\n";
	    } else
		tstring = "NULL\n";
	    ND_PRINT("%s", tstring);
	    fmt++;
	    while (ND_ASCII_ISDIGIT(*fmt))
		fmt++;
	    break;
	  }
	default:
	    ND_PRINT("%c", *fmt);
	    fmt++;
	    break;
	}
    }
    if (buf >= maxbuf && *fmt)
	ND_PRINT("END OF BUFFER\n");
    return(buf);
trunc:
    nd_print_trunc(ndo);
    return(NULL);
}