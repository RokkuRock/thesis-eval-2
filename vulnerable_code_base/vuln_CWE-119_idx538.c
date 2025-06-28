void jpc_qmfb_join_colgrp(jpc_fix_t *a, int numrows, int stride,
  int parity)
{
	int bufsize = JPC_CEILDIVPOW2(numrows, 1);
	jpc_fix_t joinbuf[QMFB_JOINBUFSIZE * JPC_QMFB_COLGRPSIZE];
	jpc_fix_t *buf = joinbuf;
	jpc_fix_t *srcptr;
	jpc_fix_t *dstptr;
	register jpc_fix_t *srcptr2;
	register jpc_fix_t *dstptr2;
	register int n;
	register int i;
	int hstartcol;
	if (bufsize > QMFB_JOINBUFSIZE) {
		if (!(buf = jas_alloc3(bufsize, JPC_QMFB_COLGRPSIZE, sizeof(jpc_fix_t)))) {
			abort();
		}
	}
	hstartcol = (numrows + 1 - parity) >> 1;
	n = hstartcol;
	srcptr = &a[0];
	dstptr = buf;
	while (n-- > 0) {
		dstptr2 = dstptr;
		srcptr2 = srcptr;
		for (i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
			*dstptr2 = *srcptr2;
			++dstptr2;
			++srcptr2;
		}
		srcptr += stride;
		dstptr += JPC_QMFB_COLGRPSIZE;
	}
	srcptr = &a[hstartcol * stride];
	dstptr = &a[(1 - parity) * stride];
	n = numrows - hstartcol;
	while (n-- > 0) {
		dstptr2 = dstptr;
		srcptr2 = srcptr;
		for (i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
			*dstptr2 = *srcptr2;
			++dstptr2;
			++srcptr2;
		}
		dstptr += 2 * stride;
		srcptr += stride;
	}
	srcptr = buf;
	dstptr = &a[parity * stride];
	n = hstartcol;
	while (n-- > 0) {
		dstptr2 = dstptr;
		srcptr2 = srcptr;
		for (i = 0; i < JPC_QMFB_COLGRPSIZE; ++i) {
			*dstptr2 = *srcptr2;
			++dstptr2;
			++srcptr2;
		}
		dstptr += 2 * stride;
		srcptr += JPC_QMFB_COLGRPSIZE;
	}
	if (buf != joinbuf) {
		jas_free(buf);
	}
}