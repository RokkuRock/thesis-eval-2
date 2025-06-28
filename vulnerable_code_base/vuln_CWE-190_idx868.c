static void ep4_mul_pre_ordin(ep4_t *t, const ep4_t p) {
	int i;
	ep4_dbl(t[0], p);
#if defined(EP_MIXED)
	ep4_norm(t[0], t[0]);
#endif
#if EP_DEPTH > 2
	ep4_add(t[1], t[0], p);
	for (i = 2; i < (1 << (EP_DEPTH - 2)); i++) {
		ep4_add(t[i], t[i - 1], t[0]);
	}
#if defined(EP_MIXED)
	for (i = 1; i < (1 << (EP_DEPTH - 2)); i++) {
		ep4_norm(t[i], t[i]);
	}
#endif
#endif
	ep4_copy(t[0], p);
}