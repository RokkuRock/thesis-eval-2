static inline int process_numeric_entity(const char **buf, unsigned *code_point)
{
	long code_l;
	int hexadecimal = (**buf == 'x' || **buf == 'X');  
	char *endptr;
	if (hexadecimal && (**buf != '\0'))
		(*buf)++;
	if ((hexadecimal && !isxdigit(**buf)) ||
			(!hexadecimal && !isdigit(**buf))) {
		return FAILURE;
	}
	code_l = strtol(*buf, &endptr, hexadecimal ? 16 : 10);
	*buf = endptr;
	if (**buf != ';')
		return FAILURE;
	if (code_l > 0x10FFFFL)
		return FAILURE;
	if (code_point != NULL)
		*code_point = (unsigned)code_l;
	return SUCCESS;
}