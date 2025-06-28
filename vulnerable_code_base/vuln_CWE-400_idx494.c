struct r_bin_pe_addr_t *PE_(check_mingw)(RBinPEObj *pe) {
	struct r_bin_pe_addr_t* entry;
	bool sw = false;
	ut8 b[1024];
	size_t n = 0;
	if (!pe || !pe->b) {
		return 0LL;
	}
	entry = PE_(r_bin_pe_get_entrypoint) (pe);
	ZERO_FILL (b);
	if (r_buf_read_at (pe->b, entry->paddr, b, sizeof (b)) < 0) {
		pe_printf ("Warning: Cannot read entry at 0x%08"PFMT64x "\n", entry->paddr);
		free (entry);
		return NULL;
	}
	if (b[0] == 0x55 && b[1] == 0x89 && b[3] == 0x83 && b[6] == 0xc7 && b[13] == 0xff && b[19] == 0xe8) {
		sw = follow_offset (entry, pe->b, b, sizeof (b), pe->big_endian, 19);
	}
	if (b[0] == 0x83 && b[3] == 0xc7 && b[10] == 0xff && b[16] == 0xe8) {
		sw = follow_offset (entry, pe->b, b, sizeof (b), pe->big_endian, 16);
	}
	if (b[0] == 0x83 && b[3] == 0xc7 && b[13] == 0xe8 && b[18] == 0x83 && b[21] == 0xe9) {
		sw = follow_offset (entry, pe->b, b, sizeof (b), pe->big_endian, 21);
	}
	if (sw) {
		for (n = 0; n < sizeof (b) - 12; n++) {
			if (b[n] == 0xa1 && b[n + 5] == 0x89 && b[n + 8] == 0xe8) {
				sw = follow_offset (entry, pe->b, b, sizeof (b), pe->big_endian, n + 8);
				return entry;
			}
		}
	}
	free (entry);
	return NULL;
}