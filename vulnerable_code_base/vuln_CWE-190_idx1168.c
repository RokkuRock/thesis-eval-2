int bn_gen_prime_factor(bn_t a, bn_t b, int abits, int bbits) {
	bn_t t;
	int result = RLC_OK;
    if (! (bbits>abits) ) {
		return RLC_ERR;
    }
    bn_null(t);
    RLC_TRY {
        bn_new(t);
		bn_gen_prime(a, abits);
        do {
            bn_rand(t, RLC_POS, bbits - bn_bits(a));
            do {
                bn_mul(b, a, t);
                bn_add_dig(b, b, 1);
                bn_add_dig(t, t, 1);
            } while(! bn_is_prime(b));
        } while (bn_bits(b) != bbits);
    }
    RLC_CATCH_ANY {
		result = RLC_ERR;
    }
    RLC_FINALLY {
        bn_free(t);
    }
    return result;
}