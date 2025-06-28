wsemul_sun_output_control(struct wsemul_sun_emuldata *edp,
    struct wsemul_inputstate *instate)
{
	int oargs;
	int rc;
	switch (instate->inchar) {
	case '0': case '1': case '2': case '3': case '4':  
	case '5': case '6': case '7': case '8': case '9':
		if (edp->nargs > SUN_EMUL_NARGS - 1) {
			bcopy(edp->args + 1, edp->args,
			    (SUN_EMUL_NARGS - 1) * sizeof(edp->args[0]));
			edp->args[edp->nargs = SUN_EMUL_NARGS - 1] = 0;
		}
		edp->args[edp->nargs] = (edp->args[edp->nargs] * 10) +
		    (instate->inchar - '0');
		break;
	case ';':		 
		edp->nargs++;
		break;
	default:		 
		oargs = edp->nargs++;
		if (edp->nargs > SUN_EMUL_NARGS)
			edp->nargs = SUN_EMUL_NARGS;
		rc = wsemul_sun_control(edp, instate);
		if (rc != 0) {
			edp->nargs = oargs;
			return rc;
		}
		edp->state = SUN_EMUL_STATE_NORMAL;
		break;
	}
	return 0;
}