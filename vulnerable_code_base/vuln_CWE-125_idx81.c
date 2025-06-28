messageAddArgument(message *m, const char *arg)
{
	int offset;
	char *p;
	assert(m != NULL);
	if(arg == NULL)
		return;	 
	while(isspace(*arg))
		arg++;
	if(*arg == '\0')
		return;
	cli_dbgmsg("messageAddArgument, arg='%s'\n", arg);
	if(!usefulArg(arg))
		return;
	for(offset = 0; offset < m->numberOfArguments; offset++)
		if(m->mimeArguments[offset] == NULL)
			break;
		else if(strcasecmp(arg, m->mimeArguments[offset]) == 0)
			return;	 
	if(offset == m->numberOfArguments) {
		char **q;
		m->numberOfArguments++;
		q = (char **)cli_realloc(m->mimeArguments, m->numberOfArguments * sizeof(char *));
		if(q == NULL) {
			m->numberOfArguments--;
			return;
		}
		m->mimeArguments = q;
	}
	p = m->mimeArguments[offset] = rfc2231(arg);
	if(!p) {
		cli_dbgmsg("messageAddArgument, error from rfc2231()\n");
		return;
	}
	if(strchr(p, '=') == NULL) {
		if(strncmp(p, "filename", 8) == 0) {
			cli_dbgmsg("Possible data corruption fixed\n");
			p[8] = '=';
		} else {
			if(*p)
				cli_dbgmsg("messageAddArgument, '%s' contains no '='\n", p);
			free(m->mimeArguments[offset]);
			m->mimeArguments[offset] = NULL;
			return;
		}
	}
	if((strncasecmp(p, "filename=", 9) == 0) || (strncasecmp(p, "name=", 5) == 0))
		if(messageGetMimeType(m) == NOMIME) {
			cli_dbgmsg("Force mime encoding to application\n");
			messageSetMimeType(m, "application");
		}
}