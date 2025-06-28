void bn_gcd_lehme(bn_t c, const bn_t a, const bn_t b) {
	bn_t x, y, u, v, t0, t1, t2, t3;
	dig_t _x, _y, q, _q, t, _t;
	dis_t _a, _b, _c, _d;
	if (bn_is_zero(a)) {
		bn_abs(c, b);
		return;
	}
	if (bn_is_zero(b)) {
		bn_abs(c, a);
		return;
	}
	bn_null(x);
	bn_null(y);
	bn_null(u);
	bn_null(v);
	bn_null(t0);
	bn_null(t1);
	bn_null(t2);
	bn_null(t3);
	RLC_TRY {
		bn_new(x);
		bn_new(y);
		bn_new(u);
		bn_new(v);
		bn_new(t0);
		bn_new(t1);
		bn_new(t2);
		bn_new(t3);
		if (bn_cmp_abs(a, b) == RLC_GT) {
			bn_abs(x, a);
			bn_abs(y, b);
		} else {
			bn_abs(x, b);
			bn_abs(y, a);
		}
		while (y->used > 1) {
			bn_rsh(u, x, bn_bits(x) - RLC_DIG);
			_x = u->dp[0];
			bn_rsh(v, y, bn_bits(x) - RLC_DIG);
			_y = v->dp[0];
			_a = _d = 1;
			_b = _c = 0;
			t = 0;
			if (_y != 0) {
				q = _x / _y;
				t = _x % _y;
			}
			if (t >= ((dig_t)1 << (RLC_DIG / 2))) {
				while (1) {
					_q = _y / t;
					_t = _y % t;
					if (_t < ((dig_t)1 << (RLC_DIG / 2))) {
						break;
					}
					_x = _y;
					_y = t;
					t = _a - q * _c;
					_a = _c;
					_c = t;
					t = _b - q * _d;
					_b = _d;
					_d = t;
					t = _t;
					q = _q;
				}
			}
			if (_b == 0) {
				bn_mod(t0, x, y);
				bn_copy(x, y);
				bn_copy(y, t0);
			} else {
				bn_rsh(u, x, bn_bits(x) - 2 * RLC_DIG);
				bn_rsh(v, y, bn_bits(x) - 2 * RLC_DIG);
				if (_a < 0) {
					bn_mul_dig(t0, u, -_a);
					bn_neg(t0, t0);
				} else {
					bn_mul_dig(t0, u, _a);
				}
				if (_b < 0) {
					bn_mul_dig(t1, v, -_b);
					bn_neg(t1, t1);
				} else {
					bn_mul_dig(t1, v, _b);
				}
				if (_c < 0) {
					bn_mul_dig(t2, u, -_c);
					bn_neg(t2, t2);
				} else {
					bn_mul_dig(t2, u, _c);
				}
				if (_d < 0) {
					bn_mul_dig(t3, v, -_d);
					bn_neg(t3, t3);
				} else {
					bn_mul_dig(t3, v, _d);
				}
				bn_add(u, t0, t1);
				bn_add(v, t2, t3);
				bn_rsh(t0, u, bn_bits(u) - RLC_DIG);
				_x = t0->dp[0];
				bn_rsh(t1, v, bn_bits(u) - RLC_DIG);
				_y = t1->dp[0];
				t = 0;
				if (_y != 0) {
					q = _x / _y;
					t = _x % _y;
				}
				if (t >= ((dig_t)1 << RLC_DIG / 2)) {
					while (1) {
						_q = _y / t;
						_t = _y % t;
						if (_t < ((dig_t)1 << RLC_DIG / 2)) {
							break;
						}
						_x = _y;
						_y = t;
						t = _a - q * _c;
						_a = _c;
						_c = t;
						t = _b - q * _d;
						_b = _d;
						_d = t;
						t = _t;
						q = _q;
					}
				}
				if (_a < 0) {
					bn_mul_dig(t0, x, -_a);
					bn_neg(t0, t0);
				} else {
					bn_mul_dig(t0, x, _a);
				}
				if (_b < 0) {
					bn_mul_dig(t1, y, -_b);
					bn_neg(t1, t1);
				} else {
					bn_mul_dig(t1, y, _b);
				}
				if (_c < 0) {
					bn_mul_dig(t2, x, -_c);
					bn_neg(t2, t2);
				} else {
					bn_mul_dig(t2, x, _c);
				}
				if (_d < 0) {
					bn_mul_dig(t3, y, -_d);
					bn_neg(t3, t3);
				} else {
					bn_mul_dig(t3, y, _d);
				}
				bn_add(x, t0, t1);
				bn_add(y, t2, t3);
			}
		}
		bn_gcd_ext_dig(c, u, v, x, y->dp[0]);
	}
	RLC_CATCH_ANY {
		RLC_THROW(ERR_CAUGHT);
	}
	RLC_FINALLY {
		bn_free(x);
		bn_free(y);
		bn_free(u);
		bn_free(v);
		bn_free(t0);
		bn_free(t1);
		bn_free(t2);
		bn_free(t3);
	}
}