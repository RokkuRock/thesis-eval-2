wsemul_vt100_output_dcs(struct wsemul_vt100_emuldata *edp,
    struct wsemul_inputstate *instate)
{
	u_int newstate = VT100_EMUL_STATE_DCS;
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
	default:
		edp->nargs++;
		if (edp->nargs > VT100_EMUL_NARGS) {
#ifdef VT100_DEBUG
			printf("vt100: too many arguments\n");
#endif
			edp->nargs = VT100_EMUL_NARGS;
		}
		newstate = VT100_EMUL_STATE_STRING;
		switch (instate->inchar) {
		case '$':
			newstate = VT100_EMUL_STATE_DCS_DOLLAR;
			break;
		case '{':  	 
		case '!':  
		case '|':  
#ifdef VT100_PRINTNOTIMPL
			printf("DCS%c ignored\n", (char)instate->inchar);
#endif
			break;
		default:
#ifdef VT100_PRINTUNKNOWN
			printf("DCS %x (%d, %d) unknown\n", instate->inchar,
			    ARG(0), ARG(1));
#endif
			break;
		}
	}
	edp->state = newstate;
	return 0;
}