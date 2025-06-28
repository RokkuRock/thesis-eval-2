static int benaloh(void) {
	int code = RLC_ERR;
	bdpe_t pub, prv;
	bn_t a, b;
	dig_t in, out;
	uint8_t buf[RLC_BN_BITS / 8 + 1];
	size_t len;
	int result;
	bn_null(a);
	bn_null(b);
	bdpe_null(pub);
	bdpe_null(prv);
	RLC_TRY {
		bn_new(a);
		bn_new(b);
		bdpe_new(pub);
		bdpe_new(prv);
		result = cp_bdpe_gen(pub, prv, bn_get_prime(47), RLC_BN_BITS);
		TEST_CASE("benaloh encryption/decryption is correct") {
			TEST_ASSERT(result == RLC_OK, end);
			len = RLC_BN_BITS / 8 + 1;
			rand_bytes(buf, 1);
			in = buf[0] % bn_get_prime(47);
			TEST_ASSERT(cp_bdpe_enc(buf, &len, in, pub) == RLC_OK, end);
			TEST_ASSERT(cp_bdpe_dec(&out, buf, len, prv) == RLC_OK, end);
			TEST_ASSERT(in == out, end);
		} TEST_END;
		TEST_CASE("benaloh encryption/decryption is homomorphic") {
			TEST_ASSERT(result == RLC_OK, end);
			len = RLC_BN_BITS / 8 + 1;
			rand_bytes(buf, 1);
			in = buf[0] % bn_get_prime(47);
			TEST_ASSERT(cp_bdpe_enc(buf, &len, in, pub) == RLC_OK, end);
			bn_read_bin(a, buf, len);
			rand_bytes(buf, 1);
			out = (buf[0] % bn_get_prime(47));
			in = (in + out) % bn_get_prime(47);
			TEST_ASSERT(cp_bdpe_enc(buf, &len, out, pub) == RLC_OK, end);
			bn_read_bin(b, buf, len);
			bn_mul(a, a, b);
			bn_mod(a, a, pub->n);
			len = bn_size_bin(pub->n);
			bn_write_bin(buf, len, a);
			TEST_ASSERT(cp_bdpe_dec(&out, buf, len, prv) == RLC_OK, end);
			TEST_ASSERT(in == out, end);
		} TEST_END;
	} RLC_CATCH_ANY {
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
	bn_free(a);
	bn_free(b);
	bdpe_free(pub);
	bdpe_free(prv);
	return code;
}