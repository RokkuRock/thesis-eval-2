char* parse_content_length( char* buffer, char* end, int* length)
{
	int number;
	char *p;
	int  size;
	p = buffer;
	while ( p<end && (*p==' ' || *p=='\t' || (*p=='\r' && *(p+1)=='\n') ||
	(*p=='\n' && (*(p+1)==' '||*(p+1)=='\t')) ))
		p++;
	if (p==end)
		goto error;
	size = 0;
	number = 0;
	while (p<end && *p>='0' && *p<='9') {
		number = number*10 + (*p)-'0';
		if (number<0) {
			LM_ERR("number overflow at pos %d in len number [%.*s]\n",
				(int)(p-buffer),(int)(end-buffer), buffer);
			return 0;
		}
		size ++;
		p++;
	}
	if (p==end || size==0)
		goto error;
	while ( p<end && (*p==' ' || *p=='\t' ||
	(*p=='\n' && (*(p+1)==' '||*(p+1)=='\t')) ))
		p++;
	if (p==end)
		goto error;
	if ( (*(p++)!='\n') && (*(p-1)!='\r' || *(p++)!='\n' ) )
		goto error;
	*length = number;
	return p;
error:
	LM_ERR("parse error near char [%d][%c]\n",*p,*p);
	return 0;
}