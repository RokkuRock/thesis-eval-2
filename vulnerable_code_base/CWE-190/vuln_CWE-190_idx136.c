int pgx_validate(jas_stream_t *in)
{
	uchar buf[PGX_MAGICLEN];
	uint_fast32_t magic;
	int i;
	int n;
	assert(JAS_STREAM_MAXPUTBACK >= PGX_MAGICLEN);
	if ((n = jas_stream_read(in, buf, PGX_MAGICLEN)) < 0) {
		return -1;
	}
	for (i = n - 1; i >= 0; --i) {
		if (jas_stream_ungetc(in, buf[i]) == EOF) {
			return -1;
		}
	}
	if (n < PGX_MAGICLEN) {
		return -1;
	}
	magic = (buf[0] << 8) | buf[1];
	if (magic != PGX_MAGIC) {
		return -1;
	}
	return 0;
}