static int pad_pkcs1(bn_t m, int *p_len, int m_len, int k_len, int operation) {
	uint8_t *id, pad = 0;
	int len, result = RLC_ERR;
	bn_t t;
	bn_null(t);
	RLC_TRY {
		bn_new(t);
		switch (operation) {
			case RSA_ENC:
				bn_zero(m);
				bn_lsh(m, m, 8);
				bn_add_dig(m, m, RSA_PUB);
				*p_len = k_len - 3 - m_len;
				for (int i = 0; i < *p_len; i++) {
					bn_lsh(m, m, 8);
					do {
						rand_bytes(&pad, 1);
					} while (pad == 0);
					bn_add_dig(m, m, pad);
				}
				bn_lsh(m, m, (m_len + 1) * 8);
				result = RLC_OK;
				break;
			case RSA_DEC:
				m_len = k_len - 1;
				bn_rsh(t, m, 8 * m_len);
				if (bn_is_zero(t)) {
					*p_len = m_len;
					m_len--;
					bn_rsh(t, m, 8 * m_len);
					pad = (uint8_t)t->dp[0];
					if (pad == RSA_PUB) {
						do {
							m_len--;
							bn_rsh(t, m, 8 * m_len);
							pad = (uint8_t)t->dp[0];
						} while (pad != 0 && m_len > 0);
						*p_len -= (m_len - 1);
						bn_mod_2b(m, m, (k_len - *p_len) * 8);
						result = (m_len > 0 ? RLC_OK : RLC_ERR);
					}
				}
				break;
			case RSA_SIG:
				id = hash_id(MD_MAP, &len);
				bn_zero(m);
				bn_lsh(m, m, 8);
				bn_add_dig(m, m, RSA_PRV);
				*p_len = k_len - 3 - m_len - len;
				for (int i = 0; i < *p_len; i++) {
					bn_lsh(m, m, 8);
					bn_add_dig(m, m, RSA_PAD);
				}
				bn_lsh(m, m, 8 * (len + 1));
				bn_read_bin(t, id, len);
				bn_add(m, m, t);
				bn_lsh(m, m, m_len * 8);
				result = RLC_OK;
				break;
			case RSA_SIG_HASH:
				bn_zero(m);
				bn_lsh(m, m, 8);
				bn_add_dig(m, m, RSA_PRV);
				*p_len = k_len - 3 - m_len;
				for (int i = 0; i < *p_len; i++) {
					bn_lsh(m, m, 8);
					bn_add_dig(m, m, RSA_PAD);
				}
				bn_lsh(m, m, 8 * (m_len + 1));
				result = RLC_OK;
				break;
			case RSA_VER:
				m_len = k_len - 1;
				bn_rsh(t, m, 8 * m_len);
				if (bn_is_zero(t)) {
					m_len--;
					bn_rsh(t, m, 8 * m_len);
					pad = (uint8_t)t->dp[0];
					if (pad == RSA_PRV) {
						int counter = 0;
						do {
							counter++;
							m_len--;
							bn_rsh(t, m, 8 * m_len);
							pad = (uint8_t)t->dp[0];
						} while (pad == RSA_PAD && m_len > 0);
						id = hash_id(MD_MAP, &len);
						bn_rsh(t, m, 8 * m_len);
						bn_mod_2b(t, t, 8);
						if (bn_is_zero(t)) {
							m_len -= len;
							bn_rsh(t, m, 8 * m_len);
							int r = 0;
							for (int i = 0; i < len; i++) {
								pad = (uint8_t)t->dp[0];
								r |= pad ^ id[len - i - 1];
								bn_rsh(t, t, 8);
							}
							*p_len = k_len - m_len;
							bn_mod_2b(m, m, m_len * 8);
							if (r == 0 && m_len == RLC_MD_LEN && counter >= 8) {
								result = RLC_OK;
							}
						}
					}
				}
				break;
			case RSA_VER_HASH:
				m_len = k_len - 1;
				bn_rsh(t, m, 8 * m_len);
				if (bn_is_zero(t)) {
					m_len--;
					bn_rsh(t, m, 8 * m_len);
					pad = (uint8_t)t->dp[0];
					if (pad == RSA_PRV) {
						int counter = 0;
						do {
							counter++;
							m_len--;
							bn_rsh(t, m, 8 * m_len);
							pad = (uint8_t)t->dp[0];
						} while (pad == RSA_PAD && m_len > 0);
						*p_len = k_len - m_len;
						bn_rsh(t, m, 8 * m_len);
						bn_mod_2b(t, t, 8);
						if (bn_is_zero(t)) {
							bn_mod_2b(m, m, m_len * 8);
							if (m_len == RLC_MD_LEN && counter >= 8) {
								result = RLC_OK;
							}
						}
					}
				}
				break;
		}
	}
	RLC_CATCH_ANY {
		result = RLC_ERR;
	}
	RLC_FINALLY {
		bn_free(t);
	}
	return result;
}