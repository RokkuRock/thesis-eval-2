void fb_print(const fb_t a) {
	int i;
	(void)a;
	for (i = RLC_FB_DIGS - 1; i > 0; i--) {
		util_print_dig(a[i], 1);
		util_print(" ");
	}
	util_print_dig(a[0], 1);
	util_print("\n");
}