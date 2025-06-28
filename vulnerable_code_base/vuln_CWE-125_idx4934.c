next_line(struct archive_read *a,
    const char **b, ssize_t *avail, ssize_t *ravail, ssize_t *nl)
{
	ssize_t len;
	int quit;
	quit = 0;
	if (*avail == 0) {
		*nl = 0;
		len = 0;
	} else
		len = get_line_size(*b, *avail, nl);
	while (*nl == 0 && len == *avail && !quit) {
		ssize_t diff = *ravail - *avail;
		size_t nbytes_req = (*ravail+1023) & ~1023U;
		ssize_t tested;
		if (nbytes_req < (size_t)*ravail + 160)
			nbytes_req <<= 1;
		*b = __archive_read_ahead(a, nbytes_req, avail);
		if (*b == NULL) {
			if (*ravail >= *avail)
				return (0);
			*b = __archive_read_ahead(a, *avail, avail);
			quit = 1;
		}
		*ravail = *avail;
		*b += diff;
		*avail -= diff;
		tested = len; 
		len = get_line_size(*b, *avail, nl);
		if (len >= 0)
			len += tested;
	}
	return (len);
}