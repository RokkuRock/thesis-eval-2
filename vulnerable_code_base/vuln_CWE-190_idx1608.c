#define N	2			 
static int psi(void) {
	int result, code = RLC_ERR;
	bn_t g, n, q, r, p[M], x[M], v[N], w[N], y[N], z[M];
	g1_t u[M], ss;
	g2_t d[M + 1], s[M + 1];
	gt_t t[M];
	crt_t crt;
	size_t l;
	bn_null(g);
	bn_null(n);
	bn_null(q);
	bn_null(r);
	g1_null(ss);
	crt_null(crt);
	RLC_TRY {
		bn_new(g);
		bn_new(n);
		bn_new(q);
		bn_new(r);
		g1_new(ss);
		for (int i = 0; i < M; i++) {
			bn_null(p[i]);
			bn_null(x[i]);
			bn_null(z[i]);
			g2_null(d[i]);
			g2_null(s[i]);
			bn_new(p[i]);
			bn_new(x[i]);
			bn_new(z[i]);
			g2_new(d[i]);
			g2_new(s[i]);
		}
		g2_null(s[M]);
		g2_new(s[M]);
		g2_null(d[M]);
		g2_new(d[M]);
		for (int i = 0; i < N; i++) {
			bn_null(v[i]);
			bn_null(w[i]);
			bn_null(y[i]);
			g1_null(u[i]);
			gt_null(t[i]);
			bn_new(v[i]);
			bn_new(w[i]);
			bn_new(y[i]);
			g1_new(u[i]);
			gt_new(t[i]);
		}
		crt_new(crt);
		result = cp_rsapsi_gen(g, n, RLC_BN_BITS);
		TEST_CASE("factoring-based laconic private set intersection is correct") {
			TEST_ASSERT(result == RLC_OK, end);
			for (int j = 0; j < M; j++) {
				bn_rand_mod(x[j], n);
			}
			for (int j = 0; j < N; j++) {
				bn_rand_mod(y[j], n);
			}
			TEST_ASSERT(cp_rsapsi_ask(q, r, p, g, n, x, M) == RLC_OK, end);
			for (int k = 0; k <= N; k++) {
				for (int j = 0; j < k; j++) {
					bn_copy(y[j], x[j]);
				}
				TEST_ASSERT(cp_rsapsi_ans(v, w, q, g, n, y, N) == RLC_OK, end);
				TEST_ASSERT(cp_rsapsi_int(z, &l, r, p, n, x, M, v, w, N) == RLC_OK, end);
				TEST_ASSERT(l == k, end);
			}
		} TEST_END;
		result = cp_shipsi_gen(g, crt, RLC_BN_BITS);
		TEST_CASE("factoring-based size-hiding private set intersection is correct") {
			TEST_ASSERT(result == RLC_OK, end);
			for (int j = 0; j < M; j++) {
				bn_rand_mod(x[j], crt->n);
			}
			for (int j = 0; j < N; j++) {
				bn_rand_mod(y[j], crt->n);
			}
			TEST_ASSERT(cp_shipsi_ask(q, r, p, g, crt->n, x, M) == RLC_OK, end);
			for (int k = 0; k <= N; k++) {
				for (int j = 0; j < k; j++) {
					bn_copy(y[j], x[j]);
				}
				TEST_ASSERT(cp_shipsi_ans(v, w[0], q, g, crt, y, N) == RLC_OK,
					end);
				TEST_ASSERT(cp_shipsi_int(z, &l, r, p, crt->n, x, M, v, w[0],
					N) == RLC_OK, end);
				TEST_ASSERT(l == k, end);
			}
		} TEST_END;
		TEST_CASE("pairing-based laconic private set intersection is correct") {
			pc_get_ord(q);
			for (int j = 0; j < M; j++) {
				bn_rand_mod(x[j], q);
			}
			for (int j = 0; j < N; j++) {
				bn_rand_mod(y[j], q);
			}
			TEST_ASSERT(cp_pbpsi_gen(q, ss, s, M) == RLC_OK, end);
			TEST_ASSERT(cp_pbpsi_ask(d, r, x, s, M) == RLC_OK, end);
			for (int k = 0; k <= N; k++) {
				for (int j = 0; j < k; j++) {
					bn_copy(y[j], x[j]);
				}
				TEST_ASSERT(cp_pbpsi_ans(t, u, ss, d[0], y, N) == RLC_OK, end);
				TEST_ASSERT(cp_pbpsi_int(z, &l, d, x, M, t, u, N) == RLC_OK,
					end);
				TEST_ASSERT(l == k, end);
			}
		} TEST_END;
	}
	RLC_CATCH_ANY {
		RLC_ERROR(end);
	}
	code = RLC_OK;
  end:
  	bn_free(g);
	bn_free(n);
    bn_free(q);
	bn_free(r);
	g1_free(ss);
	for (int i = 0; i < M; i++) {
		bn_free(p[i]);
		bn_free(x[i]);
		bn_free(z[i]);
		g2_free(d[i]);
		g2_free(s[i]);
	}
	g2_free(d[M]);
	g2_free(s[M]);
	for (int i = 0; i < N; i++) {
		bn_free(v[i]);
		bn_free(w[i]);
		bn_free(y[i]);
		g1_free(u[i]);
		gt_free(t[i]);
	}
	crt_free(crt);