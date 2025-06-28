didset_options2(void)
{
    (void)highlight_changed();
    check_opt_wim();
    (void)set_chars_option(curwin, &curwin->w_p_lcs);
    (void)set_chars_option(curwin, &p_fcs);
#ifdef FEAT_CLIPBOARD
    (void)check_clipboard_option();
#endif
#ifdef FEAT_VARTABS
    vim_free(curbuf->b_p_vsts_array);
    tabstop_set(curbuf->b_p_vsts, &curbuf->b_p_vsts_array);
    vim_free(curbuf->b_p_vts_array);
    tabstop_set(curbuf->b_p_vts,  &curbuf->b_p_vts_array);
#endif
}