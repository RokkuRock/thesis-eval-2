R_API size_t r_str_ansi_strip(char *str) {
	size_t i = 0;
	while (str[i]) {
		size_t chlen = __str_ansi_length (str + i);
		if (chlen > 1) {
			r_str_cpy (str + i + 1, str + i + chlen);
		}
		i++;
	}
	return i;
}