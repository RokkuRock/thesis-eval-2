find_start_brace(void)	     
{
    pos_T	cursor_save;
    pos_T	*trypos;
    pos_T	*pos;
    static pos_T	pos_copy;
    cursor_save = curwin->w_cursor;
    while ((trypos = findmatchlimit(NULL, '{', FM_BLOCKSTOP, 0)) != NULL)
    {
	pos_copy = *trypos;	 
	trypos = &pos_copy;
	curwin->w_cursor = *trypos;
	pos = NULL;
	if ((colnr_T)cin_skip2pos(trypos) == trypos->col
		       && (pos = ind_find_start_CORS(NULL)) == NULL)  
	    break;
	if (pos != NULL)
	    curwin->w_cursor.lnum = pos->lnum;
    }
    curwin->w_cursor = cursor_save;
    return trypos;
}