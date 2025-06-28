read_viminfo_barline(vir_T *virp, int got_encoding, int force, int writing)
{
    char_u	*p = virp->vir_line + 1;
    int		bartype;
    garray_T	values;
    bval_T	*vp;
    int		i;
    int		read_next = TRUE;
    if (*p == '<')
    {
	if (writing)
	    ga_add_string(&virp->vir_barlines, virp->vir_line);
    }
    else
    {
	ga_init2(&values, sizeof(bval_T), 20);
	bartype = getdigits(&p);
	switch (bartype)
	{
	    case BARTYPE_VERSION:
		if (!got_encoding)
		{
		    read_next = barline_parse(virp, p, &values);
		    vp = (bval_T *)values.ga_data;
		    if (values.ga_len > 0 && vp->bv_type == BVAL_NR)
			virp->vir_version = vp->bv_nr;
		}
		break;
	    case BARTYPE_HISTORY:
		read_next = barline_parse(virp, p, &values);
		handle_viminfo_history(&values, writing);
		break;
	    case BARTYPE_REGISTER:
		read_next = barline_parse(virp, p, &values);
		handle_viminfo_register(&values, force);
		break;
	    case BARTYPE_MARK:
		read_next = barline_parse(virp, p, &values);
		handle_viminfo_mark(&values, force);
		break;
	    default:
		if (writing)
		    ga_add_string(&virp->vir_barlines, virp->vir_line);
	}
	for (i = 0; i < values.ga_len; ++i)
	{
	    vp = (bval_T *)values.ga_data + i;
	    if (vp->bv_type == BVAL_STRING && vp->bv_allocated)
		vim_free(vp->bv_string);
	    vim_free(vp->bv_tofree);
	}
	ga_clear(&values);
    }
    if (read_next)
	return viminfo_readline(virp);
    return FALSE;
}