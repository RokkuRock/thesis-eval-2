struct r_bin_pe_addr_t *PE_(check_unknow)(RBinPEObj *pe) {
	struct r_bin_pe_addr_t *entry;
	if (!pe || !pe->b) {
		return 0LL;
	}
	ut8 b[512];
	ZERO_FILL (b);
	entry = PE_ (r_bin_pe_get_entrypoint) (pe);
	if (r_buf_read_at (pe->b, entry->paddr, b, 512) < 1) {
		pe_printf ("Warning: Cannot read entry at 0x%08"PFMT64x"\n", entry->paddr);
		free (entry);
		return NULL;
	}
	if (b[367] == 0xe8) {
		follow_offset (entry, pe->b, b, sizeof (b), pe->big_endian, 367);
		return entry;
	}
	size_t i;
	for (i = 0; i < 512 - 16 ; i++) {
		if (!memcmp (b + i, "\xff\x15", 2)) {
			if (b[i + 6] == 0x50) {
				if (b[i + 7] == 0xe8) {
					follow_offset (entry, pe->b, b, sizeof (b), pe->big_endian, i + 7);
					return entry;
				}
			}
		}
	}
	free (entry);
	return NULL;
}