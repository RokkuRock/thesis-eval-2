wsemul_vt100_output_csi(struct wsemul_vt100_emuldata *edp,
    struct wsemul_inputstate *instate)
{
	u_int newstate = VT100_EMUL_STATE_CSI;
	int oargs;
	int rc = 0;
	switch (instate->inchar) {
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
		if (edp->nargs > VT100_EMUL_NARGS - 1)
			break;
		edp->args[edp->nargs] = (edp->args[edp->nargs] * 10) +
		    (instate->inchar - '0');
		break;
	case ';':  
		edp->nargs++;
		break;
	case '?':  
	case '>':  
		edp->modif1 = (char)instate->inchar;
		break;
	case '!':
	case '"':
	case '$':
	case '&':
		edp->modif2 = (char)instate->inchar;
		break;
	default:  
		oargs = edp->nargs++;
		if (edp->nargs > VT100_EMUL_NARGS) {
#ifdef VT100_DEBUG
			printf("vt100: too many arguments\n");
#endif
			edp->nargs = VT100_EMUL_NARGS;
		}
		rc = wsemul_vt100_handle_csi(edp, instate);
		if (rc != 0) {
			edp->nargs = oargs;
			return rc;
		}
		newstate = VT100_EMUL_STATE_NORMAL;
		break;
	}
	if (COLS_LEFT != 0)
		edp->flags &= ~VTFL_LASTCHAR;
	edp->state = newstate;
	return 0;
}