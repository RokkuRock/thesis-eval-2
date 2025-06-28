	consume_count(type)
		const char **type;
{
	int count = 0;
	if (!isdigit((unsigned char)**type))
		return -1;
	while (isdigit((unsigned char)**type)) {
		count *= 10;
		if ((count % 10) != 0) {
			while (isdigit((unsigned char)**type))
				(*type)++;
			return -1;
		}
		count += **type - '0';
		(*type)++;
	}
	return (count);
}