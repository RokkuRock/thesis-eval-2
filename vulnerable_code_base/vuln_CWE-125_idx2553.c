static const char *parse_string(cJSON *item,const char *str,const char **ep)
{
	const char *ptr=str+1,*end_ptr=str+1;char *ptr2;char *out;int len=0;unsigned uc,uc2;
	if (*str!='\"') {*ep=str;return 0;}	 
	while (*end_ptr!='\"' && *end_ptr && ++len) if (*end_ptr++ == '\\') end_ptr++;	 
	out=(char*)cJSON_malloc(len+1);	 
	if (!out) return 0;
	item->valuestring=out;  
	item->type=cJSON_String;
	ptr=str+1;ptr2=out;
	while (ptr < end_ptr)
	{
		if (*ptr!='\\') *ptr2++=*ptr++;
		else
		{
			ptr++;
			switch (*ptr)
			{
				case 'b': *ptr2++='\b';	break;
				case 'f': *ptr2++='\f';	break;
				case 'n': *ptr2++='\n';	break;
				case 'r': *ptr2++='\r';	break;
				case 't': *ptr2++='\t';	break;
				case 'u':	  
					uc=parse_hex4(ptr+1);ptr+=4;	 
					if (ptr >= end_ptr) {*ep=str;return 0;}	 
					if ((uc>=0xDC00 && uc<=0xDFFF) || uc==0)    {*ep=str;return 0;}	 
					if (uc>=0xD800 && uc<=0xDBFF)	 
					{
						if (ptr+6 > end_ptr)    {*ep=str;return 0;}	 
						if (ptr[1]!='\\' || ptr[2]!='u')    {*ep=str;return 0;}	 
						uc2=parse_hex4(ptr+3);ptr+=6;
						if (uc2<0xDC00 || uc2>0xDFFF)       {*ep=str;return 0;}	 
						uc=0x10000 + (((uc&0x3FF)<<10) | (uc2&0x3FF));
					}
					len=4;if (uc<0x80) len=1;else if (uc<0x800) len=2;else if (uc<0x10000) len=3; ptr2+=len;
					switch (len) {
						case 4: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
						case 3: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
						case 2: *--ptr2 =((uc | 0x80) & 0xBF); uc >>= 6;
						case 1: *--ptr2 =(uc | firstByteMark[len]);
					}
					ptr2+=len;
					break;
				default:  *ptr2++=*ptr; break;
			}
			ptr++;
		}
	}
	*ptr2=0;
	if (*ptr=='\"') ptr++;
	return ptr;
}