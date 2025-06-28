movemark(int count)
{
    pos_T	*pos;
    xfmark_T	*jmp;
    cleanup_jumplist(curwin, TRUE);
    if (curwin->w_jumplistlen == 0)	     
	return (pos_T *)NULL;
    for (;;)
    {
	if (curwin->w_jumplistidx + count < 0
		|| curwin->w_jumplistidx + count >= curwin->w_jumplistlen)
	    return (pos_T *)NULL;
	if (curwin->w_jumplistidx == curwin->w_jumplistlen)
	{
	    setpcmark();
	    --curwin->w_jumplistidx;	 
	    if (curwin->w_jumplistidx + count < 0)
		return (pos_T *)NULL;
	}
	curwin->w_jumplistidx += count;
	jmp = curwin->w_jumplist + curwin->w_jumplistidx;
	if (jmp->fmark.fnum == 0)
	    fname2fnum(jmp);
	if (jmp->fmark.fnum != curbuf->b_fnum)
	{
	    if (buflist_findnr(jmp->fmark.fnum) == NULL)
	    {					      
		count += count < 0 ? -1 : 1;
		continue;
	    }
	    if (buflist_getfile(jmp->fmark.fnum, jmp->fmark.mark.lnum,
							    0, FALSE) == FAIL)
		return (pos_T *)NULL;
	    curwin->w_cursor = jmp->fmark.mark;
	    pos = (pos_T *)-1;
	}
	else
	    pos = &(jmp->fmark.mark);
	return pos;
    }
}