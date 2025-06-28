process_pfa(FILE *ifp, const char *ifp_filename, struct font_reader *fr)
{
    char buffer[LINESIZE];
    int c = 0;
    int blocktyp = PFA_ASCII;
    char saved_orphan = 0;
    (void)ifp_filename;
    while (c != EOF) {
	char *line = buffer, *last = buffer;
	int crlf = 0;
	c = getc(ifp);
	while (c != EOF && c != '\r' && c != '\n' && last < buffer + LINESIZE - 1) {
	    *last++ = c;
	    c = getc(ifp);
	}
	if (last == buffer + LINESIZE - 1)
	    ungetc(c, ifp);
	else if (c == '\r' && blocktyp != PFA_BINARY) {
	    c = getc(ifp);
	    if (c != '\n')
		ungetc(c, ifp), crlf = 1;
	    else
		crlf = 2;
	    *last++ = '\n';
	} else if (c != EOF)
	    *last++ = c;
	*last = 0;
	if (blocktyp == PFA_ASCII) {
	    if (strncmp(line, "currentfile eexec", 17) == 0 && isspace(line[17])) {
		char saved_p;
		for (line += 18; isspace(*line); line++)
		     ;
		saved_p = *line;
		*line = 0;
		fr->output_ascii(buffer, line - buffer);
		*line = saved_p;
		blocktyp = PFA_EEXEC_TEST;
		if (!*line)
		    continue;
	    } else {
		fr->output_ascii(line, last - line);
		continue;
	    }
	}
	if (blocktyp == PFA_EEXEC_TEST) {
	    for (; line < last && isspace(*line); line++)
		 ;
	    if (line == last)
		continue;
	    else if (last >= line + 4 && isxdigit(line[0]) && isxdigit(line[1])
		     && isxdigit(line[2]) && isxdigit(line[3]))
		blocktyp = PFA_HEX;
	    else
		blocktyp = PFA_BINARY;
	    memmove(buffer, line, last - line + 1);
	    last = buffer + (last - line);
	    line = buffer;
	    if (blocktyp == PFA_BINARY && crlf) {
		last[-1] = '\r';
		if (crlf == 2)
		    *last++ = '\n';
	    }
	}
	if (all_zeroes(line)) {	 
	    fr->output_ascii(line, last - line);
	    blocktyp = PFA_ASCII;
	} else if (blocktyp == PFA_HEX) {
	    int len = translate_hex_string(line, &saved_orphan);
	    if (len)
		fr->output_binary((unsigned char *)line, len);
	} else
	    fr->output_binary((unsigned char *)line, last - line);
    }
    fr->output_end();
}