int print_calc(
    image_desc_t *im)
{
    long      i, ii, validsteps;
    double    printval;
    struct tm tmvdef;
    int       graphelement = 0;
    long      vidx;
    int       max_ii;
    double    magfact = -1;
    char     *si_symb = "";
    char     *percent_s;
    int       prline_cnt = 0;
    time_t    now = time(NULL);
    localtime_r(&now, &tmvdef);
    for (i = 0; i < im->gdes_c; i++) {
        vidx = im->gdes[i].vidx;
        switch (im->gdes[i].gf) {
        case GF_PRINT:
        case GF_GPRINT:
            if (im->gdes[vidx].gf == GF_VDEF) {  
                printval = im->gdes[vidx].vf.val;
                localtime_r(&im->gdes[vidx].vf.when, &tmvdef);
            } else {     
                max_ii = ((im->gdes[vidx].end - im->gdes[vidx].start)
                          / im->gdes[vidx].step * im->gdes[vidx].ds_cnt);
                printval = DNAN;
                validsteps = 0;
                for (ii = im->gdes[vidx].ds;
                     ii < max_ii; ii += im->gdes[vidx].ds_cnt) {
                    if (!finite(im->gdes[vidx].data[ii]))
                        continue;
                    if (isnan(printval)) {
                        printval = im->gdes[vidx].data[ii];
                        validsteps++;
                        continue;
                    }
                    switch (im->gdes[i].cf) {
                    case CF_HWPREDICT:
                    case CF_MHWPREDICT:
                    case CF_DEVPREDICT:
                    case CF_DEVSEASONAL:
                    case CF_SEASONAL:
                    case CF_AVERAGE:
                        validsteps++;
                        printval += im->gdes[vidx].data[ii];
                        break;
                    case CF_MINIMUM:
                        printval = min(printval, im->gdes[vidx].data[ii]);
                        break;
                    case CF_FAILURES:
                    case CF_MAXIMUM:
                        printval = max(printval, im->gdes[vidx].data[ii]);
                        break;
                    case CF_LAST:
                        printval = im->gdes[vidx].data[ii];
                    }
                }
                if (im->gdes[i].cf == CF_AVERAGE || im->gdes[i].cf > CF_LAST) {
                    if (validsteps > 1) {
                        printval = (printval / validsteps);
                    }
                }
            }            
            if (!im->gdes[i].strftm && (percent_s = strstr(im->gdes[i].format, "%S")) != NULL) {
                if (magfact < 0.0) {
                    auto_scale(im, &printval, &si_symb, &magfact);
                    if (printval == 0.0)
                        magfact = -1.0;
                } else {
                    printval /= magfact;
                }
                *(++percent_s) = 's';
            } else if (!im->gdes[i].strftm && strstr(im->gdes[i].format, "%s") != NULL) {
                auto_scale(im, &printval, &si_symb, &magfact);
            }
            if (im->gdes[i].gf == GF_PRINT) {
                rrd_infoval_t prline;
                if (im->gdes[i].strftm) {
                    prline.u_str = (char*)malloc((FMT_LEG_LEN + 2) * sizeof(char));
                    if (im->gdes[vidx].vf.never == 1) {
                       time_clean(prline.u_str, im->gdes[i].format);
                    } else {
                        strftime(prline.u_str,
                                 FMT_LEG_LEN, im->gdes[i].format, &tmvdef);
                    }
                } else if (bad_format(im->gdes[i].format)) {
                    rrd_set_error
                        ("bad format for PRINT in '%s'", im->gdes[i].format);
                    return -1;
                } else {
                    prline.u_str =
                        sprintf_alloc(im->gdes[i].format, printval, si_symb);
                }
                grinfo_push(im,
                            sprintf_alloc
                            ("print[%ld]", prline_cnt++), RD_I_STR, prline);
                free(prline.u_str);
            } else {
                if (im->gdes[i].strftm) {
                    if (im->gdes[vidx].vf.never == 1) {
                       time_clean(im->gdes[i].legend, im->gdes[i].format);
                    } else {
                        strftime(im->gdes[i].legend,
                                 FMT_LEG_LEN, im->gdes[i].format, &tmvdef);
                    }
                } else {
                    if (bad_format(im->gdes[i].format)) {
                        rrd_set_error
                            ("bad format for GPRINT in '%s'",
                             im->gdes[i].format);
                        return -1;
                    }
                    snprintf(im->gdes[i].legend,
                             FMT_LEG_LEN - 2,
                             im->gdes[i].format, printval, si_symb);
                }
                graphelement = 1;
            }
            break;
        case GF_LINE:
        case GF_AREA:
		case GF_GRAD:
        case GF_TICK:
            graphelement = 1;
            break;
        case GF_HRULE:
            if (isnan(im->gdes[i].yrule)) {  
                im->gdes[i].yrule = im->gdes[vidx].vf.val;
            };
            graphelement = 1;
            break;
        case GF_VRULE:
            if (im->gdes[i].xrule == 0) {    
                im->gdes[i].xrule = im->gdes[vidx].vf.when;
            };
            graphelement = 1;
            break;
        case GF_COMMENT:
        case GF_TEXTALIGN:
        case GF_DEF:
        case GF_CDEF:
        case GF_VDEF:
#ifdef WITH_PIECHART
        case GF_PART:
#endif
        case GF_SHIFT:
        case GF_XPORT:
            break;
        case GF_STACK:
            rrd_set_error
                ("STACK should already be turned into LINE or AREA here");
            return -1;
            break;
        case GF_XAXIS:
        case GF_YAXIS:
	    break;
        }
    }
    return graphelement;
}