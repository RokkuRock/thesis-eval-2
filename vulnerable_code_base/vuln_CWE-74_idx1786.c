int valid_field (const char *field, const char *illegal)
{
	const char *cp;
	int err = 0;
	if (NULL == field) {
		return -1;
	}
	for (cp = field; '\0' != *cp; cp++) {
		if (strchr (illegal, *cp) != NULL) {
			err = -1;
			break;
		}
	}
	if (0 == err) {
		for (cp = field; '\0' != *cp; cp++) {
			if (!isprint (*cp)) {
				err = 1;
				break;
			}
		}
	}
	return err;
}