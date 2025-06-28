parse_range(char *str, size_t file_sz, int *nranges)
{
	static struct range	 ranges[MAX_RANGES];
	int			 i = 0;
	char			*p, *q;
	if ((p = strchr(str, '=')) == NULL)
		return (NULL);
	*p++ = '\0';
	if (strcmp(str, "bytes") != 0)
		return (NULL);
	while ((q = strchr(p, ',')) != NULL) {
		*q++ = '\0';
		if (parse_range_spec(p, file_sz, &ranges[i]) == 0)
			continue;
		i++;
		if (i == MAX_RANGES)
			return (NULL);
		p = q;
	}
	if (parse_range_spec(p, file_sz, &ranges[i]) != 0)
		i++;
	*nranges = i;
	return (i ? ranges : NULL);
}