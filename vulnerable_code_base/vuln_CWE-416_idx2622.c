is_qf_win(win_T *win, qf_info_T *qi)
{
    if (bt_quickfix(win->w_buffer))
	if ((IS_QF_STACK(qi) && win->w_llist_ref == NULL)
		|| (IS_LL_STACK(qi) && win->w_llist_ref == qi))
	    return TRUE;
    return FALSE;
}