int bn_smb_jac(const bn_t a, const bn_t b) {
	bn_t t0, t1, r;
	int t, h, res;
	bn_null(t0);
	bn_null(t1);
	bn_null(r);
	if (bn_is_even(b) || bn_sign(b) == RLC_NEG) {
		RLC_THROW(ERR_NO_VALID);
		return 0;
	}
	RLC_TRY {
		bn_new(t0);
		bn_new(t1);
		bn_new(r);
		t = 1;
		if (bn_sign(a) == RLC_NEG) {
			bn_add(t0, a, b);
		} else {
			bn_copy(t0, a);
		}
		bn_copy(t1, b);
		while (1) {
			bn_mod(t0, t0, t1);
			if (bn_is_zero(t0)) {
				if (bn_cmp_dig(t1, 1) == RLC_EQ) {
					res = 1;
					if (t == -1) {
						res = -1;
					}
					break;
				} else {
					res = 0;
					break;
				}
			}
			h = 0;
			while (bn_is_even(t0)) {
				h++;
				bn_rsh(t0, t0, 1);
			}
			bn_mod_2b(r, t1, 3);
			if ((h % 2 != 0) && (bn_cmp_dig(r, 1) != RLC_EQ) &&
					(bn_cmp_dig(r, 7) != RLC_EQ)) {
				t = -t;
			}
			bn_mod_2b(r, t0, 2);
			if (bn_cmp_dig(r, 1) != RLC_EQ) {
				bn_mod_2b(r, t1, 2);
				if (bn_cmp_dig(r, 1) != RLC_EQ) {
					t = -t;
				}
			}
			bn_copy(r, t0);
			bn_copy(t0, t1);
			bn_copy(t1, r);
		}
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(t0);
		bn_free(t1);
		bn_free(r);
	}
	return res;
}