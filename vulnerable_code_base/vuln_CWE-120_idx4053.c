PJ_DEF(void) pj_scan_skip_line( pj_scanner *scanner )
{
    char *s = pj_memchr(scanner->curptr, '\n', scanner->end - scanner->curptr);
    if (!s) {
	scanner->curptr = scanner->end;
    } else {
	scanner->curptr = scanner->start_line = s+1;
	scanner->line++;
   }
}