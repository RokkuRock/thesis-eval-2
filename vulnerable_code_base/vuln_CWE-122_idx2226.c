check_text_locked(oparg_T *oap)
{
    if (text_locked())
    {
	clearopbeep(oap);
	text_locked_msg();
	return TRUE;
    }
    return FALSE;
}