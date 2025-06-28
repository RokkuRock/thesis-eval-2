int cp_rsa_gen(rsa_t pub, rsa_t prv, int bits) {
	bn_t t, r;
	int result = RLC_OK;
	if (pub == NULL || prv == NULL || bits == 0) {
		return RLC_ERR;
	}
	bn_null(t);
	bn_null(r);
	RLC_TRY {
		bn_new(t);
		bn_new(r);
		do {
			bn_gen_prime(prv->crt->p, bits / 2);
			bn_gen_prime(prv->crt->q, bits / 2);
		} while (bn_cmp(prv->crt->p, prv->crt->q) == RLC_EQ);
		if (bn_cmp(prv->crt->p, prv->crt->q) != RLC_LT) {
			bn_copy(t, prv->crt->p);
			bn_copy(prv->crt->p, prv->crt->q);
			bn_copy(prv->crt->q, t);
		}
		bn_mul(pub->crt->n, prv->crt->p, prv->crt->q);
		bn_copy(prv->crt->n, pub->crt->n);
		bn_sub_dig(prv->crt->p, prv->crt->p, 1);
		bn_sub_dig(prv->crt->q, prv->crt->q, 1);
		bn_mul(t, prv->crt->p, prv->crt->q);
		bn_set_2b(pub->e, 16);
		bn_add_dig(pub->e, pub->e, 1);
#if !defined(CP_CRT)
		bn_gcd_ext(r, prv->d, NULL, pub->e, t);
		if (bn_sign(prv->d) == RLC_NEG) {
			bn_add(prv->d, prv->d, t);
		}
		if (bn_cmp_dig(r, 1) == RLC_EQ) {
			bn_add_dig(prv->crt->p, prv->crt->p, 1);
			bn_add_dig(prv->crt->q, prv->crt->q, 1);
			result = RLC_OK;
		}
#else
		bn_gcd_ext(r, prv->d, NULL, pub->e, t);
		if (bn_sign(prv->d) == RLC_NEG) {
			bn_add(prv->d, prv->d, t);
		}
		if (bn_cmp_dig(r, 1) == RLC_EQ) {
			bn_mod(prv->crt->dp, prv->d, prv->crt->p);
			bn_mod(prv->crt->dq, prv->d, prv->crt->q);
			bn_add_dig(prv->crt->p, prv->crt->p, 1);
			bn_add_dig(prv->crt->q, prv->crt->q, 1);
			bn_mod_inv(prv->crt->qi, prv->crt->q, prv->crt->p);
			result = RLC_OK;
		}
#endif  
	}
	RLC_CATCH_ANY {
		result = RLC_ERR;
	}
	RLC_FINALLY {
		bn_free(t);
		bn_free(r);
	}
	return result;
}