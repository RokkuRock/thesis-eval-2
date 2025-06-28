win_new_width(win_T *wp, int width)
{
    wp->w_width = width;
    wp->w_lines_valid = 0;
    changed_line_abv_curs_win(wp);
    if (p_spsc)
    {
	invalidate_botline_win(wp);
	if (wp == curwin)
	{
	    update_topline();
	    curs_columns(TRUE);	 
	}
    }
    redraw_win_later(wp, UPD_NOT_VALID);
    wp->w_redr_status = TRUE;
}