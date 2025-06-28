long jpc_bitstream_getbits(jpc_bitstream_t *bitstream, int n)
{
	long v;
	int u;
	assert(n >= 0 && n < 32);
	v = 0;
	while (--n >= 0) {
		if ((u = jpc_bitstream_getbit(bitstream)) < 0) {
			return -1;
		}
		v = (v << 1) | u;
	}
	return v;
}