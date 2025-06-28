ins_compl_get_exp(pos_T *ini)
{
    static ins_compl_next_state_T st;
    int		i;
    int		found_new_match;
    int		type = ctrl_x_mode;
    if (!compl_started)
    {
	FOR_ALL_BUFFERS(st.ins_buf)
	    st.ins_buf->b_scanned = 0;
	st.found_all = FALSE;
	st.ins_buf = curbuf;
	st.e_cpt = (compl_cont_status & CONT_LOCAL)
					    ? (char_u *)"." : curbuf->b_p_cpt;
	st.last_match_pos = st.first_match_pos = *ini;
    }
    else if (st.ins_buf != curbuf && !buf_valid(st.ins_buf))
	st.ins_buf = curbuf;   
    compl_old_match = compl_curr_match;	 
    st.cur_match_pos = (compl_dir_forward())
				? &st.last_match_pos : &st.first_match_pos;
    for (;;)
    {
	found_new_match = FAIL;
	st.set_match_pos = FALSE;
	if ((ctrl_x_mode_normal() || ctrl_x_mode_line_or_eval())
					&& (!compl_started || st.found_all))
	{
	    int status = process_next_cpt_value(&st, &type, ini);
	    if (status == INS_COMPL_CPT_END)
		break;
	    if (status == INS_COMPL_CPT_CONT)
		continue;
	}
	if (compl_pattern == NULL)
	    break;
	found_new_match = get_next_completion_match(type, &st, ini);
	if ((ctrl_x_mode_not_default() && !ctrl_x_mode_line_or_eval())
						|| found_new_match != FAIL)
	{
	    if (got_int)
		break;
	    if (type != -1)
		ins_compl_check_keys(0, FALSE);
	    if ((ctrl_x_mode_not_default()
			&& !ctrl_x_mode_line_or_eval()) || compl_interrupted)
		break;
	    compl_started = TRUE;
	}
	else
	{
	    if (type == 0 || type == CTRL_X_PATH_PATTERNS)
		st.ins_buf->b_scanned = TRUE;
	    compl_started = FALSE;
	}
    }
    compl_started = TRUE;
    if ((ctrl_x_mode_normal() || ctrl_x_mode_line_or_eval())
	    && *st.e_cpt == NUL)		 
	found_new_match = FAIL;
    i = -1;		 
    if (found_new_match == FAIL || (ctrl_x_mode_not_default()
					       && !ctrl_x_mode_line_or_eval()))
	i = ins_compl_make_cyclic();
    if (compl_old_match != NULL)
    {
	compl_curr_match = compl_dir_forward() ? compl_old_match->cp_next
						    : compl_old_match->cp_prev;
	if (compl_curr_match == NULL)
	    compl_curr_match = compl_old_match;
    }
    may_trigger_modechanged();
    return i;
}