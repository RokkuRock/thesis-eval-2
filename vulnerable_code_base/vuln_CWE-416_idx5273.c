qf_jump_newwin(qf_info_T	*qi,
	int		dir,
	int		errornr,
	int		forceit,
	int		newwin)
{
    qf_list_T		*qfl;
    qfline_T		*qf_ptr;
    qfline_T		*old_qf_ptr;
    int			qf_index;
    int			old_qf_index;
    char_u		*old_swb = p_swb;
    unsigned		old_swb_flags = swb_flags;
    int			prev_winid;
    int			opened_window = FALSE;
    int			print_message = TRUE;
    int			old_KeyTyped = KeyTyped;  
    int			retval = OK;
    if (qi == NULL)
	qi = &ql_info;
    if (qf_stack_empty(qi) || qf_list_empty(qf_get_curlist(qi)))
    {
	emsg(_(e_no_errors));
	return;
    }
    incr_quickfix_busy();
    qfl = qf_get_curlist(qi);
    qf_ptr = qfl->qf_ptr;
    old_qf_ptr = qf_ptr;
    qf_index = qfl->qf_index;
    old_qf_index = qf_index;
    qf_ptr = qf_get_entry(qfl, errornr, dir, &qf_index);
    if (qf_ptr == NULL)
    {
	qf_ptr = old_qf_ptr;
	qf_index = old_qf_index;
	goto theend;
    }
    qfl->qf_index = qf_index;
    qfl->qf_ptr = qf_ptr;
    if (qf_win_pos_update(qi, old_qf_index))
	print_message = FALSE;
    prev_winid = curwin->w_id;
    retval = qf_jump_open_window(qi, qf_ptr, newwin, &opened_window);
    if (retval == FAIL)
	goto failed;
    if (retval == NOTDONE)
	goto theend;
    retval = qf_jump_to_buffer(qi, qf_index, qf_ptr, forceit, prev_winid,
				  &opened_window, old_KeyTyped, print_message);
    if (retval == NOTDONE)
    {
	qi = NULL;
	qf_ptr = NULL;
    }
    if (retval != OK)
    {
	if (opened_window)
	    win_close(curwin, TRUE);     
	if (qf_ptr != NULL && qf_ptr->qf_fnum != 0)
	{
failed:
	    qf_ptr = old_qf_ptr;
	    qf_index = old_qf_index;
	}
    }
theend:
    if (qi != NULL)
    {
	qfl->qf_ptr = qf_ptr;
	qfl->qf_index = qf_index;
    }
    if (p_swb != old_swb && p_swb == empty_option)
    {
	p_swb = old_swb;
	swb_flags = old_swb_flags;
    }
    decr_quickfix_busy();
}