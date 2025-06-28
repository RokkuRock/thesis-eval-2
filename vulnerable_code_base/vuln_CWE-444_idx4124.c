int h1_parse_cont_len_header(struct h1m *h1m, struct ist *value)
{
	char *e, *n;
	long long cl;
	int not_first = !!(h1m->flags & H1_MF_CLEN);
	struct ist word;
	word.ptr = value->ptr - 1;  
	e = value->ptr + value->len;
	while (++word.ptr < e) {
		if (unlikely(HTTP_IS_LWS(*word.ptr)))
			continue;
		for (cl = 0, n = word.ptr; n < e; n++) {
			unsigned int c = *n - '0';
			if (unlikely(c > 9)) {
				if (unlikely(n == word.ptr))  
					goto fail;
				break;
			}
			if (unlikely(cl > ULLONG_MAX / 10ULL))
				goto fail;  
			cl = cl * 10ULL;
			if (unlikely(cl + c < cl))
				goto fail;  
			cl = cl + c;
		}
		word.len = n - word.ptr;
		for (; n < e; n++) {
			if (!HTTP_IS_LWS(*n)) {
				if (unlikely(*n != ','))
					goto fail;
				break;
			}
		}
		if (h1m->flags & H1_MF_CLEN && cl != h1m->body_len)
			goto fail;
		h1m->flags |= H1_MF_CLEN;
		h1m->curr_len = h1m->body_len = cl;
		*value = word;
		word.ptr = n;
	}
	return !not_first;
 fail:
	return -1;
}