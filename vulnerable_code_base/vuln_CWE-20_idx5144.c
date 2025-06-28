int mif_validate(jas_stream_t *in)
{
	uchar buf[MIF_MAGICLEN];
	uint_fast32_t magic;
	int i;
	int n;
	assert(JAS_STREAM_MAXPUTBACK >= MIF_MAGICLEN);
	if ((n = jas_stream_read(in, buf, MIF_MAGICLEN)) < 0) {
		return -1;
	}
	for (i = n - 1; i >= 0; --i) {
		if (jas_stream_ungetc(in, buf[i]) == EOF) {
			return -1;
		}
	}
	if (n < MIF_MAGICLEN) {
		return -1;
	}
	magic = (JAS_CAST(uint_fast32_t, buf[0]) << 24) |
	  (JAS_CAST(uint_fast32_t, buf[1]) << 16) |
	  (JAS_CAST(uint_fast32_t, buf[2]) << 8) |
	  buf[3];
	if (magic != MIF_MAGIC) {
		return -1;
	}
	return 0;
}