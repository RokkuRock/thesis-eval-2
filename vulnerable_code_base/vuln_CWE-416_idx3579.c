can_unload_buffer(buf_T *buf)
{
    int	    can_unload = !buf->b_locked;
    if (can_unload && updating_screen)
    {
	win_T	*wp;
	FOR_ALL_WINDOWS(wp)
	    if (wp->w_buffer == buf)
	    {
		can_unload = FALSE;
		break;
	    }
    }
    if (!can_unload)
	semsg(_(e_attempt_to_delete_buffer_that_is_in_use_str), buf->b_fname);
    return can_unload;
}