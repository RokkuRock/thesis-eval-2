static int getid(char ch) {
	const char *keys = "[]<>+-,.";
	const char *cidx = strchr (keys, ch);
	return cidx? cidx - keys + 1: 0;
}