int delete_sdp_line( struct sip_msg * msg, char * s)
{
	char * start,*end;
	if( !s )
		return 1;
	start = s;
	end  = s;
	while(*start != '\n')
		start--;
	start++;
	while(*end != '\n')
		end++;
	end++;
	if( del_lump(msg, start - msg->buf, end - start,0) == NULL )
	{
		return -1;
	}
	return 0;
}