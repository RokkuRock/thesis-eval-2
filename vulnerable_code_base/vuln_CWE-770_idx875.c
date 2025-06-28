static int stream_process(struct sip_msg * msg, struct sdp_stream_cell *cell,
			str * s, str* ss, regex_t* re, int op,int description)
{
	static sdp_payload_attr_t static_payloads[] = {
	{ NULL,0,{ "0",1},{"PCMU",4},{ "8000",4},{NULL,0},{NULL,0} },    
	{ NULL,0,{ "3",1},{ "GSM",3},{ "8000",4},{NULL,0},{NULL,0} },    
	{ NULL,0,{ "4",1},{"G723",4},{ "8000",4},{NULL,0},{NULL,0} },    
	{ NULL,0,{ "5",1},{"DVI4",4},{ "8000",4},{NULL,0},{NULL,0} },    
	{ NULL,0,{ "6",1},{"DVI4",4},{"16000",5},{NULL,0},{NULL,0} },    
	{ NULL,0,{ "7",1},{ "LPC",3},{ "8000",4},{NULL,0},{NULL,0} },    
	{ NULL,0,{ "8",1},{"PCMA",4},{ "8000",4},{NULL,0},{NULL,0} },    
	{ NULL,0,{ "9",1},{"G722",4},{ "8000",4},{NULL,0},{NULL,0} },    
	{ NULL,0,{"10",2},{ "L16",3},{"44100",5},{NULL,0},{NULL,0} },    
	{ NULL,0,{"11",2},{ "L16",3},{"44100",5},{NULL,0},{NULL,0} },    
	{ NULL,0,{"12",2},{"QCELP",5},{"8000",4},{NULL,0},{NULL,0} },    
	{ NULL,0,{"13",2},{  "CN",2},{ "8000",4},{NULL,0},{NULL,0} },    
	{ NULL,0,{"14",2},{ "MPA",3},{"90000",5},{NULL,0},{NULL,0} },    
	{ NULL,0,{"15",2},{"G728",4},{ "8000",4},{NULL,0},{NULL,0} },    
	{ NULL,0,{"16",2},{"DVI4",4},{"11025",5},{NULL,0},{NULL,0} },    
	{ NULL,0,{"17",2},{"DVI4",4},{"22050",5},{NULL,0},{NULL,0} },    
	{ NULL,0,{"18",2},{"G729",4},{ "8000",4},{NULL,0},{NULL,0} },    
	{ NULL,0,{"25",2},{"CelB",4},{ "8000",4},{NULL,0},{NULL,0} },    
	{ NULL,0,{"26",2},{"JPEG",4},{"90000",5},{NULL,0},{NULL,0} },    
	{ NULL,0,{"28",2},{  "nv",2},{"90000",5},{NULL,0},{NULL,0} },    
	{ NULL,0,{"31",2},{"H261",4},{"90000",5},{NULL,0},{NULL,0} },    
	{ NULL,0,{"32",2},{ "MPV",3},{"90000",5},{NULL,0},{NULL,0} },    
	{ NULL,0,{"33",2},{"MP2T",4},{"90000",5},{NULL,0},{NULL,0} },    
	{ NULL,0,{"34",2},{"H263",4},{"90000",5},{NULL,0},{NULL,0} },    
	{ NULL,0,{"t38",3},{"t38",3},{     "",0},{NULL,0},{NULL,0} },    
	{ NULL,0,{NULL,0},{  NULL,0},{   NULL,0},{NULL,0},{NULL,0} }
	};
	sdp_payload_attr_t *payload;
	char *cur, *tmp, *buff, temp;
	struct lump * lmp;
	str found;
	int ret, i,match, buff_len, is_static;
	regmatch_t pmatch;
	lmp = get_associated_lump(msg, cell);
	if( lmp == NULL)
	{
		LM_ERR("There is no lump for this sdp cell\n");
		return -1;
	}
	if (lmp->len == 0)
		return -1;
	buff_len = 0;
	ret = 0;
	buff = pkg_malloc(lmp->len+1);
	if( buff == NULL)
	{
		LM_ERR("Out of memory\n");
		return -1;
	}
	is_static = 0;
	payload = cell->payload_attr;
	while(payload)
	{
		if( payload->rtp_enc.s == NULL
		 || (payload->rtp_clock.s == NULL && ss != NULL)
		 || payload->rtp_payload.s == NULL)
		{
			goto next_payload;
		}
		match = 0;
		if( description == DESC_REGEXP ||description == DESC_REGEXP_COMPLEMENT )
		{
			if (is_static) {
				match = regexec( re, payload->rtp_enc.s, 1, &pmatch, 0) == 0;
			} else {
				temp = payload->rtp_enc.s[payload->rtp_enc.len];
				payload->rtp_enc.s[payload->rtp_enc.len] = 0;
				match = regexec( re, payload->rtp_enc.s, 1, &pmatch, 0) == 0;
				payload->rtp_enc.s[payload->rtp_enc.len] = temp;
			}
		}
		if( description == DESC_REGEXP_COMPLEMENT)
			match = !match;
		if( description == DESC_NAME  )
		{
			match = s->len == payload->rtp_enc.len &&
			strncasecmp( s->s, payload->rtp_enc.s ,	payload->rtp_enc.len) == 0;
		}
		if( description == DESC_NAME_AND_CLOCK)
		{
			match = s->len == payload->rtp_enc.len &&
			strncasecmp( s->s, payload->rtp_enc.s ,
				payload->rtp_enc.len) == 0
			&&
			(ss == NULL || ( ss->len == payload->rtp_clock.len &&
			strncasecmp( ss->s, payload->rtp_clock.s ,
				payload->rtp_clock.len) == 0
			) );
		}
		if (match) {
			match = 0;
			cur = lmp->u.value;
			while( !match && cur < lmp->u.value + lmp->len)
			{
				found.s = cur;
				while(  cur < lmp->u.value + lmp->len &&  *cur != ' ' )
					cur++;
				found.len = cur - found.s;
				if ( found.len == payload->rtp_payload.len &&
				strncmp( found.s,payload->rtp_payload.s,found.len) == 0) {
					match = 1;
				} else {
					while( cur < lmp->u.value + lmp->len && * cur == ' '  )
						cur++;
				}
			}
			if (match) {
				if(op == FIND)
				{
					ret = 1;
					goto end;
				}
				if( op == DELETE && !is_static )
				{
					if( delete_sdp_line( msg, payload->rtp_enc.s, cell) < 0 )
					{
						LM_ERR("Unable to add delete lump for a=\n");
						ret = -1;
						goto end;
					}
					if( delete_sdp_line( msg, payload->fmtp_string.s, cell) < 0 )
					{
						LM_ERR("Unable to add delete lump for a=\n");
						ret = -1;
						goto end;
					}
				}
				{
					while (found.s > lmp->u.value && *(found.s - 1) == ' ') {
						found.s--;
						found.len++;
					}
					if (cur == lmp->u.value + lmp->len) {
						tmp = found.s;
						while (*(--tmp) == ' ') {
							found.s--;
							found.len++;
						}
					}
					for(tmp=found.s ; tmp< lmp->u.value + lmp->len ; tmp++ )
						*tmp  = *(tmp+found.len);
					lmp->len -= found.len;
				}
				if( op == ADD_TO_FRONT  || op == ADD_TO_BACK)
				{
					memcpy(&buff[buff_len]," ",1);
					buff_len++;
					memcpy(&buff[buff_len],payload->rtp_payload.s,
						payload->rtp_payload.len);
					buff_len += payload->rtp_payload.len;
				}
				ret = 1;
			}
		}
	next_payload:
		if (!is_static) {
			payload = payload->next;
			if (payload==NULL) {
				payload = static_payloads;
				is_static = 1;
			}
		} else {
			payload ++;
			if (payload->rtp_payload.s==NULL)
				payload=NULL;
		}
	}
	if( op == ADD_TO_FRONT && buff_len >0 )
	{
		lmp->u.value = (char*)pkg_realloc(lmp->u.value, lmp->len+buff_len);
		if(!lmp->u.value) {
			LM_ERR("No more pkg memory\n");
			ret = -1;
			goto end;
		}
		for( i = lmp->len -1 ; i>=0;i--)
			lmp->u.value[i+buff_len] = lmp->u.value[i];
		memcpy(lmp->u.value,buff,buff_len);
		lmp->len += buff_len;
	}
	if( op == ADD_TO_BACK && buff_len >0 )
	{
		lmp->u.value = (char*)pkg_realloc(lmp->u.value, lmp->len+buff_len);
		if(!lmp->u.value) {
			LM_ERR("No more pkg memory\n");
			ret = -1;
			goto end;
		}
		memcpy(&lmp->u.value[lmp->len],buff,buff_len);
		lmp->len += buff_len;
	}
	if (lmp->len == 0) {
		lmp = del_lump(msg, cell->port.s - msg->buf - 1, cell->port.len + 2, 0);
		if (!lmp) {
			LM_ERR("could not add lump to disable stream!\n");
			goto end;
		}
		tmp = pkg_malloc(3);
		if (!tmp) {
			LM_ERR("oom for port 0\n");
			goto end;
		}
		memcpy(tmp, " 0 ", 3);
		if (!insert_new_lump_after(lmp, tmp, 3, 0))
			LM_ERR("could not insert lump to disable stream!\n");
	}
end:
	pkg_free(buff);
	return ret;
}