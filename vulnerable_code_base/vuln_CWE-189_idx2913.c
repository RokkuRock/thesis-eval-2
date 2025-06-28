queryin(char *buf)
{
	QPRS_STATE	state;
	int32		i;
	ltxtquery  *query;
	int32		commonlen;
	ITEM	   *ptr;
	NODE	   *tmp;
	int32		pos = 0;
#ifdef BS_DEBUG
	char		pbuf[16384],
			   *cur;
#endif
	state.buf = buf;
	state.state = WAITOPERAND;
	state.count = 0;
	state.num = 0;
	state.str = NULL;
	state.sumlen = 0;
	state.lenop = 64;
	state.curop = state.op = (char *) palloc(state.lenop);
	*(state.curop) = '\0';
	makepol(&state);
	if (!state.num)
		ereport(ERROR,
				(errcode(ERRCODE_SYNTAX_ERROR),
				 errmsg("syntax error"),
				 errdetail("Empty query.")));
	commonlen = COMPUTESIZE(state.num, state.sumlen);
	query = (ltxtquery *) palloc(commonlen);
	SET_VARSIZE(query, commonlen);
	query->size = state.num;
	ptr = GETQUERY(query);
	for (i = 0; i < state.num; i++)
	{
		ptr[i].type = state.str->type;
		ptr[i].val = state.str->val;
		ptr[i].distance = state.str->distance;
		ptr[i].length = state.str->length;
		ptr[i].flag = state.str->flag;
		tmp = state.str->next;
		pfree(state.str);
		state.str = tmp;
	}
	memcpy((void *) GETOPERAND(query), (void *) state.op, state.sumlen);
	pfree(state.op);
	pos = 0;
	findoprnd(ptr, &pos);
	return query;
}