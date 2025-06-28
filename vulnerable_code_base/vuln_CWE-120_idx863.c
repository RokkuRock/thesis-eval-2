parse_user_name(char *user_input, char **ret_username)
{
	register char *ptr;
	register int index = 0;
	char username[PAM_MAX_RESP_SIZE];
	*ret_username = NULL;
	bzero((void *)username, PAM_MAX_RESP_SIZE);
	ptr = user_input;
	while ((*ptr == ' ') || (*ptr == '\t'))
		ptr++;
	if (*ptr == '\0') {
		return (PAM_BUF_ERR);
	}
	while (*ptr != '\0') {
		if ((*ptr == ' ') || (*ptr == '\t'))
			break;
		else {
			username[index] = *ptr;
			index++;
			ptr++;
		}
	}
	if ((*ret_username = malloc(index + 1)) == NULL)
		return (PAM_BUF_ERR);
	(void) strcpy(*ret_username, username);
	return (PAM_SUCCESS);
}