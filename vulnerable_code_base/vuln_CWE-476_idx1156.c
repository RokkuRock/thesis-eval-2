static void jpc_undo_roi(jas_matrix_t *x, int roishift, int bgshift, int numbps)
{
	int i;
	int j;
	int thresh;
	jpc_fix_t val;
	jpc_fix_t mag;
	bool warn;
	uint_fast32_t mask;
	if (roishift == 0 && bgshift == 0) {
		return;
	}
	thresh = 1 << roishift;
	warn = false;
	for (i = 0; i < jas_matrix_numrows(x); ++i) {
		for (j = 0; j < jas_matrix_numcols(x); ++j) {
			val = jas_matrix_get(x, i, j);
			mag = JAS_ABS(val);
			if (mag >= thresh) {
				mag >>= roishift;
				val = (val < 0) ? (-mag) : mag;
				jas_matrix_set(x, i, j, val);
			} else {
				mag <<= bgshift;
				mask = (1 << numbps) - 1;
				if (mag & (~mask)) {
					if (!warn) {
						jas_eprintf("warning: possibly corrupt code stream\n");
						warn = true;
					}
					mag &= mask;
				}
				val = (val < 0) ? (-mag) : mag;
				jas_matrix_set(x, i, j, val);
			}
		}
	}
}