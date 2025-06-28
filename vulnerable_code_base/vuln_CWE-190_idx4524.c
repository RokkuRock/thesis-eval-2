int cp_sokaka_key(uint8_t *key, size_t key_len, const char *id1,
		const sokaka_t k, const char *id2) {
	int len1 = strlen(id1), len2 = strlen(id2);
	int size, first = 0, result = RLC_OK;
	uint8_t *buf;
	g1_t p;
	g2_t q;
	gt_t e;
	g1_null(p);
	g2_null(q);
	gt_null(e);
	RLC_TRY {
		g1_new(p);
		g2_new(q);
		gt_new(e);
		size = gt_size_bin(e, 0);
		buf = RLC_ALLOCA(uint8_t, size);
		if (buf == NULL) {
			RLC_THROW(ERR_NO_MEMORY);
		}
		if (len1 == len2) {
			if (strncmp(id1, id2, len1) == 0) {
				RLC_THROW(ERR_NO_VALID);
			}
			first = (strncmp(id1, id2, len1) < 0 ? 1 : 2);
		} else {
			if (len1 < len2) {
				if (strncmp(id1, id2, len1) == 0) {
					first = 1;
				} else {
					first = (strncmp(id1, id2, len1) < 0 ? 1 : 2);
				}
			} else {
				if (strncmp(id1, id2, len2) == 0) {
					first = 2;
				} else {
					first = (strncmp(id1, id2, len2) < 0 ? 1 : 2);
				}
			}
		}
		if (pc_map_is_type1()) {
			g2_map(q, (uint8_t *)id2, len2);
			pc_map(e, k->s1, q);
		} else {
			if (first == 1) {
				g2_map(q, (uint8_t *)id2, len2);
				pc_map(e, k->s1, q);
			} else {
				g1_map(p, (uint8_t *)id2, len2);
				pc_map(e, p, k->s2);
			}
		}
		gt_write_bin(buf, size, e, 0);
		md_kdf(key, key_len, buf, size);
	}
	RLC_CATCH_ANY {
		result = RLC_ERR;
	}
	RLC_FINALLY {
		g1_free(p);
		g2_free(q);
		gt_free(e);
		RLC_FREE(buf);
	}
	return result;
}