PJ_DEF(void) pj_scan_get_newline( pj_scanner *scanner )
{
    if (!PJ_SCAN_IS_NEWLINE(*scanner->curptr)) {
	pj_scan_syntax_err(scanner);
	return;
    }
    if (*scanner->curptr == '\r') {
	++scanner->curptr;
    }
    if (*scanner->curptr == '\n') {
	++scanner->curptr;
    }
    ++scanner->line;
    scanner->start_line = scanner->curptr;
}