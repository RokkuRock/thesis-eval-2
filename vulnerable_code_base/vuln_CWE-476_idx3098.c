buflist_match(
    regmatch_T	*rmp,
    buf_T	*buf,
    int		ignore_case)   
{
    char_u	*match;
    match = fname_match(rmp, buf->b_sfname, ignore_case);
    if (match == NULL)
	match = fname_match(rmp, buf->b_ffname, ignore_case);
    return match;
}