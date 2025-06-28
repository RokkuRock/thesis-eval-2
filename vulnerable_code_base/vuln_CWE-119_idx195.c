safe_fprintf(FILE *f, const char *fmt, ...)
{
	char fmtbuff_stack[256];  
	char outbuff[256];  
	char *fmtbuff_heap;  
	char *fmtbuff;   
	int fmtbuff_length;
	int length, n;
	va_list ap;
	const char *p;
	unsigned i;
	wchar_t wc;
	char try_wc;
	fmtbuff_heap = NULL;
	fmtbuff_length = sizeof(fmtbuff_stack);
	fmtbuff = fmtbuff_stack;
	va_start(ap, fmt);
	length = vsnprintf(fmtbuff, fmtbuff_length, fmt, ap);
	va_end(ap);
	while (length < 0 || length >= fmtbuff_length) {
		if (length >= fmtbuff_length)
			fmtbuff_length = length+1;
		else if (fmtbuff_length < 8192)
			fmtbuff_length *= 2;
		else if (fmtbuff_length < 1000000)
			fmtbuff_length += fmtbuff_length / 4;
		else {
			length = fmtbuff_length;
			fmtbuff_heap[length-1] = '\0';
			break;
		}
		free(fmtbuff_heap);
		fmtbuff_heap = malloc(fmtbuff_length);
		if (fmtbuff_heap != NULL) {
			fmtbuff = fmtbuff_heap;
			va_start(ap, fmt);
			length = vsnprintf(fmtbuff, fmtbuff_length, fmt, ap);
			va_end(ap);
		} else {
			length = sizeof(fmtbuff_stack) - 1;
			break;
		}
	}
	if (mbtowc(NULL, NULL, 1) == -1) {  
		free(fmtbuff_heap);
		return;
	}
	p = fmtbuff;
	i = 0;
	try_wc = 1;
	while (*p != '\0') {
		if (try_wc && (n = mbtowc(&wc, p, length)) != -1) {
			length -= n;
			if (iswprint(wc) && wc != L'\\') {
				while (n-- > 0)
					outbuff[i++] = *p++;
			} else {
				while (n-- > 0)
					i += (unsigned)bsdtar_expand_char(
					    outbuff, i, *p++);
			}
		} else {
			i += (unsigned)bsdtar_expand_char(outbuff, i, *p++);
			try_wc = 0;
		}
		if (i > (sizeof(outbuff) - 20)) {
			outbuff[i] = '\0';
			fprintf(f, "%s", outbuff);
			i = 0;
		}
	}
	outbuff[i] = '\0';
	fprintf(f, "%s", outbuff);
	free(fmtbuff_heap);
}