int jpc_bitstream_putbits(jpc_bitstream_t *bitstream, int n, long v)
{
	int m;
	assert(n >= 0 && n < 32);
	assert(!(v & (~JAS_ONES(n))));
	m = n - 1;
	while (--n >= 0) {
		if (jpc_bitstream_putbit(bitstream, (v >> m) & 1) == EOF) {
			return EOF;
		}
		v <<= 1;
	}
	return 0;
}