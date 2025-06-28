parse_tsquery(char *buf,
			  PushFunction pushval,
			  Datum opaque,
			  bool isplain)
{
	struct TSQueryParserStateData state;
	int			i;
	TSQuery		query;
	int			commonlen;
	QueryItem  *ptr;
	ListCell   *cell;
	state.buffer = buf;
	state.buf = buf;
	state.state = (isplain) ? WAITSINGLEOPERAND : WAITFIRSTOPERAND;
	state.count = 0;
	state.polstr = NIL;
	state.valstate = init_tsvector_parser(state.buffer, true, true);
	state.sumlen = 0;
	state.lenop = 64;
	state.curop = state.op = (char *) palloc(state.lenop);
	*(state.curop) = '\0';
	makepol(&state, pushval, opaque);
	close_tsvector_parser(state.valstate);
	if (list_length(state.polstr) == 0)
	{
		ereport(NOTICE,
				(errmsg("text-search query doesn't contain lexemes: \"%s\"",
						state.buffer)));
		query = (TSQuery) palloc(HDRSIZETQ);
		SET_VARSIZE(query, HDRSIZETQ);
		query->size = 0;
		return query;
	}
	commonlen = COMPUTESIZE(list_length(state.polstr), state.sumlen);
	query = (TSQuery) palloc0(commonlen);
	SET_VARSIZE(query, commonlen);
	query->size = list_length(state.polstr);
	ptr = GETQUERY(query);
	i = 0;
	foreach(cell, state.polstr)
	{
		QueryItem  *item = (QueryItem *) lfirst(cell);
		switch (item->type)
		{
			case QI_VAL:
				memcpy(&ptr[i], item, sizeof(QueryOperand));
				break;
			case QI_VALSTOP:
				ptr[i].type = QI_VALSTOP;
				break;
			case QI_OPR:
				memcpy(&ptr[i], item, sizeof(QueryOperator));
				break;
			default:
				elog(ERROR, "unrecognized QueryItem type: %d", item->type);
		}
		i++;
	}
	memcpy((void *) GETOPERAND(query), (void *) state.op, state.sumlen);
	pfree(state.op);
	findoprnd(ptr, query->size);
	return query;
}