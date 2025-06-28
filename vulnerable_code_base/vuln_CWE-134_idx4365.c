void rrd_graph_options(
    int argc,
    char *argv[],
    image_desc_t
    *im)
{
    int       stroff;
    char     *parsetime_error = NULL;
    char      scan_gtm[12], scan_mtm[12], scan_ltm[12], col_nam[12];
    char      double_str[20], double_str2[20];
    time_t    start_tmp = 0, end_tmp = 0;
    long      long_tmp;
    rrd_time_value_t start_tv, end_tv;
    long unsigned int color;
#define LONGOPT_UNITS_SI 255
    struct option long_options[] = {
        { "alt-autoscale",      no_argument,       0, 'A'},
        { "imgformat",          required_argument, 0, 'a'},
        { "font-smoothing-threshold", required_argument, 0, 'B'},
        { "base",               required_argument, 0, 'b'},
        { "color",              required_argument, 0, 'c'},
        { "full-size-mode",     no_argument,       0, 'D'},
        { "daemon",             required_argument, 0, 'd'},
        { "slope-mode",         no_argument,       0, 'E'},
        { "end",                required_argument, 0, 'e'},
        { "force-rules-legend", no_argument,       0, 'F'},
        { "imginfo",            required_argument, 0, 'f'},
        { "graph-render-mode",  required_argument, 0, 'G'},
        { "no-legend",          no_argument,       0, 'g'},
        { "height",             required_argument, 0, 'h'},
        { "no-minor",           no_argument,       0, 'I'},
        { "interlaced",         no_argument,       0, 'i'},
        { "alt-autoscale-min",  no_argument,       0, 'J'},
        { "only-graph",         no_argument,       0, 'j'},
        { "units-length",       required_argument, 0, 'L'},
        { "lower-limit",        required_argument, 0, 'l'},
        { "alt-autoscale-max",  no_argument,       0, 'M'},
        { "zoom",               required_argument, 0, 'm'},
        { "no-gridfit",         no_argument,       0, 'N'},
        { "font",               required_argument, 0, 'n'},
        { "logarithmic",        no_argument,       0, 'o'},
        { "pango-markup",       no_argument,       0, 'P'},
        { "font-render-mode",   required_argument, 0, 'R'},
        { "rigid",              no_argument,       0, 'r'},
        { "step",               required_argument, 0, 'S'},
        { "start",              required_argument, 0, 's'},
        { "tabwidth",           required_argument, 0, 'T'},
        { "title",              required_argument, 0, 't'},
        { "upper-limit",        required_argument, 0, 'u'},
        { "vertical-label",     required_argument, 0, 'v'},
        { "watermark",          required_argument, 0, 'W'},
        { "width",              required_argument, 0, 'w'},
        { "units-exponent",     required_argument, 0, 'X'},
        { "x-grid",             required_argument, 0, 'x'},
        { "alt-y-grid",         no_argument,       0, 'Y'},
        { "y-grid",             required_argument, 0, 'y'},
        { "lazy",               no_argument,       0, 'z'},
        { "use-nan-for-all-missing-data", no_argument,       0, 'Z'},
        { "units",              required_argument, 0, LONGOPT_UNITS_SI},
        { "alt-y-mrtg",         no_argument,       0, 1000},     
        { "disable-rrdtool-tag",no_argument,       0, 1001},
        { "right-axis",         required_argument, 0, 1002},
        { "right-axis-label",   required_argument, 0, 1003},
        { "right-axis-format",  required_argument, 0, 1004},
        { "legend-position",    required_argument, 0, 1005},
        { "legend-direction",   required_argument, 0, 1006},
        { "border",             required_argument, 0, 1007},
        { "grid-dash",          required_argument, 0, 1008},
        { "dynamic-labels",     no_argument,       0, 1009},
        { "week-fmt",           required_argument, 0, 1010},
        { "graph-type",         required_argument, 0, 1011},
        { "left-axis-format",   required_argument, 0, 1012},
        {  0, 0, 0, 0}
};
    optind = 0;
    opterr = 0;          
    rrd_parsetime("end-24h", &start_tv);
    rrd_parsetime("now", &end_tv);
    while (1) {
        int       option_index = 0;
        int       opt;
        int       col_start, col_end;
        opt = getopt_long(argc, argv,
                          "Aa:B:b:c:Dd:Ee:Ff:G:gh:IiJjL:l:Mm:Nn:oPR:rS:s:T:t:u:v:W:w:X:x:Yy:Zz",
                          long_options, &option_index);
        if (opt == EOF)
            break;
        switch (opt) {
        case 'I':
            im->extra_flags |= NOMINOR;
            break;
        case 'Y':
            im->extra_flags |= ALTYGRID;
            break;
        case 'A':
            im->extra_flags |= ALTAUTOSCALE;
            break;
        case 'J':
            im->extra_flags |= ALTAUTOSCALE_MIN;
            break;
        case 'M':
            im->extra_flags |= ALTAUTOSCALE_MAX;
            break;
        case 'j':
            im->extra_flags |= ONLY_GRAPH;
            break;
        case 'g':
            im->extra_flags |= NOLEGEND;
            break;
        case 'Z':
            im->extra_flags |= ALLOW_MISSING_DS;
            break;
        case 1005:
            if (strcmp(optarg, "north") == 0) {
                im->legendposition = NORTH;
            } else if (strcmp(optarg, "west") == 0) {
                im->legendposition = WEST;
            } else if (strcmp(optarg, "south") == 0) {
                im->legendposition = SOUTH;
            } else if (strcmp(optarg, "east") == 0) {
                im->legendposition = EAST;
            } else {
                rrd_set_error("unknown legend-position '%s'", optarg);
                return;
            }
            break;
        case 1006:
            if (strcmp(optarg, "topdown") == 0) {
                im->legenddirection = TOP_DOWN;
            } else if (strcmp(optarg, "bottomup") == 0) {
                im->legenddirection = BOTTOM_UP;
            } else if (strcmp(optarg, "bottomup2") == 0) {
                im->legenddirection = BOTTOM_UP2;
            } else {
                rrd_set_error("unknown legend-position '%s'", optarg);
                return;
            }
            break;
        case 'F':
            im->extra_flags |= FORCE_RULES_LEGEND;
            break;
        case 1001:
            im->extra_flags |= NO_RRDTOOL_TAG;
            break;
        case LONGOPT_UNITS_SI:
            if (im->extra_flags & FORCE_UNITS) {
                rrd_set_error("--units can only be used once!");
                return;
            }
            if (strcmp(optarg, "si") == 0)
                im->extra_flags |= FORCE_UNITS_SI;
            else {
                rrd_set_error("invalid argument for --units: %s", optarg);
                return;
            }
            break;
        case 'X':
            im->unitsexponent = atoi(optarg);
            break;
        case 'L':
            im->unitslength = atoi(optarg);
            im->forceleftspace = 1;
            break;
        case 'T':
            if (rrd_strtodbl(optarg, 0, &(im->tabwidth), "option -T") != 2)
                return;
            break;
        case 'S':
            im->step = atoi(optarg);
            break;
        case 'N':
            im->gridfit = 0;
            break;
        case 'P':
            im->with_markup = 1;
            break;
        case 's':
            if ((parsetime_error = rrd_parsetime(optarg, &start_tv))) {
                rrd_set_error("start time: %s", parsetime_error);
                return;
            }
            break;
        case 'e':
            if ((parsetime_error = rrd_parsetime(optarg, &end_tv))) {
                rrd_set_error("end time: %s", parsetime_error);
                return;
            }
            break;
        case 'x':
            if (strcmp(optarg, "none") == 0) {
                im->draw_x_grid = 0;
                break;
            };
            if (sscanf(optarg,
                       "%10[A-Z]:%ld:%10[A-Z]:%ld:%10[A-Z]:%ld:%ld:%n",
                       scan_gtm,
                       &im->xlab_user.gridst,
                       scan_mtm,
                       &im->xlab_user.mgridst,
                       scan_ltm,
                       &im->xlab_user.labst,
                       &im->xlab_user.precis, &stroff) == 7 && stroff != 0) {
				im->xlab_form=strdup(optarg + stroff);
				if (!im->xlab_form) {
                    rrd_set_error("cannot allocate memory for xlab_form");
                    return;
				}
                if ((int)
                    (im->xlab_user.gridtm = tmt_conv(scan_gtm)) == -1) {
                    rrd_set_error("unknown keyword %s", scan_gtm);
                    return;
                } else if ((int)
                           (im->xlab_user.mgridtm = tmt_conv(scan_mtm))
                           == -1) {
                    rrd_set_error("unknown keyword %s", scan_mtm);
                    return;
                } else if ((int)
                           (im->xlab_user.labtm = tmt_conv(scan_ltm)) == -1) {
                    rrd_set_error("unknown keyword %s", scan_ltm);
                    return;
                }
                im->xlab_user.minsec = 1;
                im->xlab_user.stst = im->xlab_form ? im->xlab_form : "";
            } else {
                rrd_set_error("invalid x-grid format");
                return;
            }
            break;
        case 'y':
            if (strcmp(optarg, "none") == 0) {
                im->draw_y_grid = 0;
                break;
            };
            if (sscanf(optarg, "%[-0-9.e+]:%d", double_str , &im->ylabfact) == 2) {
                if (rrd_strtodbl( double_str, 0, &(im->ygridstep), "option -y") != 2){
                    return;
                }
                if (im->ygridstep <= 0) {
                    rrd_set_error("grid step must be > 0");
                    return;
                } else if (im->ylabfact < 1) {
                    rrd_set_error("label factor must be > 0");
                    return;
                }
            } else {
                rrd_set_error("invalid y-grid format");
                return;
            }
            break;
        case 1007:
            im->draw_3d_border = atoi(optarg);
            break;
        case 1008:  
            if(sscanf(optarg,
                      "%[-0-9.e+]:%[-0-9.e+]",
                      double_str,
                      double_str2 ) != 2) {
                if ( rrd_strtodbl( double_str, 0, &(im->grid_dash_on),NULL) !=2 
                     || rrd_strtodbl( double_str2, 0, &(im->grid_dash_off), NULL) != 2 ){
                    rrd_set_error("expected grid-dash format float:float");
                    return;
                }
            }
            break;   
        case 1009:  
            im->dynamic_labels = 1;
            break;         
        case 1010:
            strncpy(week_fmt,optarg,sizeof week_fmt);
            week_fmt[(sizeof week_fmt)-1]='\0';
            break;
        case 1002:  
            if(sscanf(optarg,
                      "%[-0-9.e+]:%[-0-9.e+]",
                      double_str,
                      double_str2 ) == 2
                && rrd_strtodbl( double_str, 0, &(im->second_axis_scale),NULL) == 2
                && rrd_strtodbl( double_str2, 0, &(im->second_axis_shift),NULL) == 2){
                if(im->second_axis_scale==0){
                    rrd_set_error("the second_axis_scale  must not be 0");
                    return;
                }
            } else {
                rrd_set_error("invalid right-axis format expected scale:shift");
                return;
            }
            break;
        case 1003:
            im->second_axis_legend=strdup(optarg);
            if (!im->second_axis_legend) {
                rrd_set_error("cannot allocate memory for second_axis_legend");
                return;
            }
            break;
        case 1004:
            if (bad_format(optarg)){
                rrd_set_error("use either %le or %lf formats");
                return;
            }
            im->second_axis_format=strdup(optarg);
            if (!im->second_axis_format) {
                rrd_set_error("cannot allocate memory for second_axis_format");
                return;
            }
            break;
        case 1012:
            if (bad_format(optarg)){
                rrd_set_error("use either %le or %lf formats");
                return;
            }
            im->primary_axis_format=strdup(optarg);
            if (!im->primary_axis_format) {
                rrd_set_error("cannot allocate memory for primary_axis_format");
                return;
            }
            break;
        case 'v':
            im->ylegend=strdup(optarg);
            if (!im->ylegend) {
                rrd_set_error("cannot allocate memory for ylegend");
                return;
            }
            break;
        case 'u':
            if (rrd_strtodbl(optarg, 0, &(im->maxval), "option -u") != 2){
                return;
            }
            break;
        case 'l':
            if (rrd_strtodbl(optarg, 0, &(im->minval), "option -l") != 2){
                return;
            }
            break;
        case 'b':
            im->base = atol(optarg);
            if (im->base != 1024 && im->base != 1000) {
                rrd_set_error
                    ("the only sensible value for base apart from 1000 is 1024");
                return;
            }
            break;
        case 'w':
            long_tmp = atol(optarg);
            if (long_tmp < 10) {
                rrd_set_error("width below 10 pixels");
                return;
            }
            im->xsize = long_tmp;
            break;
        case 'h':
            long_tmp = atol(optarg);
            if (long_tmp < 10) {
                rrd_set_error("height below 10 pixels");
                return;
            }
            im->ysize = long_tmp;
            break;
        case 'D':
            im->extra_flags |= FULL_SIZE_MODE;
            break;
        case 'i':
            break;
        case 'r':
            im->rigid = 1;
            break;
        case 'f':
            im->imginfo = optarg;
            break;
        case 'a':
            if ((int)
                (im->imgformat = if_conv(optarg)) == -1) {
                rrd_set_error("unsupported graphics format '%s'", optarg);
                return;
            }
            break;
        case 1011:
            if ((int)
                (im->graph_type = type_conv(optarg)) == -1) {
                rrd_set_error("unsupported graphics type '%s'", optarg);
                return;
            }
            break;
        case 'z':
            im->lazy = 1;
            break;
        case 'E':
            im->slopemode = 1;
            break;
        case 'o':
            im->logarithmic = 1;
            break;
        case 'c':
            if (sscanf(optarg,
                       "%10[A-Z]#%n%8lx%n",
                       col_nam, &col_start, &color, &col_end) == 2) {
                int       ci;
                int       col_len = col_end - col_start;
                switch (col_len) {
                case 3:
                    color =
                        (((color & 0xF00) * 0x110000) | ((color & 0x0F0) *
                                                         0x011000) |
                         ((color & 0x00F)
                          * 0x001100)
                         | 0x000000FF);
                    break;
                case 4:
                    color =
                        (((color & 0xF000) *
                          0x11000) | ((color & 0x0F00) *
                                      0x01100) | ((color &
                                                   0x00F0) *
                                                  0x00110) |
                         ((color & 0x000F) * 0x00011)
                        );
                    break;
                case 6:
                    color = (color << 8) + 0xff   ;
                    break;
                case 8:
                    break;
                default:
                    rrd_set_error("the color format is #RRGGBB[AA]");
                    return;
                }
                if ((ci = grc_conv(col_nam)) != -1) {
                    im->graph_col[ci] = gfx_hex_to_col(color);
                } else {
                    rrd_set_error("invalid color name '%s'", col_nam);
                    return;
                }
            } else {
                rrd_set_error("invalid color def format");
                return;
            }
            break;
        case 'n':{
            char      prop[15];
            double    size = 1;
            int       end;
            if (sscanf(optarg, "%10[A-Z]:%[-0-9.e+]%n", prop, double_str, &end) >= 2
                && rrd_strtodbl( double_str, 0, &size, NULL) == 2) {
                int       sindex, propidx;
                if ((sindex = text_prop_conv(prop)) != -1) {
                    for (propidx = sindex;
                         propidx < TEXT_PROP_LAST; propidx++) {
                        if (size > 0) {
                            rrd_set_font_desc(im,propidx,NULL,size);
                        }
                        if ((int) strlen(optarg) > end+2) {
                            if (optarg[end] == ':') {
                                rrd_set_font_desc(im,propidx,optarg + end + 1,0);
                            } else {
                                rrd_set_error
                                    ("expected : after font size in '%s'",
                                     optarg);
                                return;
                            }
                        }
                        if (propidx == sindex && sindex != 0)
                            break;
                    }
                } else {
                    rrd_set_error("invalid fonttag '%s'", prop);
                    return;
                }
            } else {
                rrd_set_error("invalid text property format");
                return;
            }
            break;
        }
        case 'm':
            if (rrd_strtodbl(optarg, 0, &(im->zoom), "option -m") != 2){
                return;
            }
            if (im->zoom <= 0.0) {
                rrd_set_error("zoom factor must be > 0");
                return;
            }
            break;
        case 't':
            im->title=strdup(optarg);
            if (!im->title) {
                rrd_set_error("cannot allocate memory for title");
                return;
            }
            break;
        case 'R':
            if (strcmp(optarg, "normal") == 0) {
                cairo_font_options_set_antialias
                    (im->font_options, CAIRO_ANTIALIAS_GRAY);
                cairo_font_options_set_hint_style
                    (im->font_options, CAIRO_HINT_STYLE_FULL);
            } else if (strcmp(optarg, "light") == 0) {
                cairo_font_options_set_antialias
                    (im->font_options, CAIRO_ANTIALIAS_GRAY);
                cairo_font_options_set_hint_style
                    (im->font_options, CAIRO_HINT_STYLE_SLIGHT);
            } else if (strcmp(optarg, "mono") == 0) {
                cairo_font_options_set_antialias
                    (im->font_options, CAIRO_ANTIALIAS_NONE);
                cairo_font_options_set_hint_style
                    (im->font_options, CAIRO_HINT_STYLE_FULL);
            } else {
                rrd_set_error("unknown font-render-mode '%s'", optarg);
                return;
            }
            break;
        case 'G':
            if (strcmp(optarg, "normal") == 0)
                im->graph_antialias = CAIRO_ANTIALIAS_GRAY;
            else if (strcmp(optarg, "mono") == 0)
                im->graph_antialias = CAIRO_ANTIALIAS_NONE;
            else {
                rrd_set_error("unknown graph-render-mode '%s'", optarg);
                return;
            }
            break;
        case 'B':
            break;
        case 'W':
            im->watermark=strdup(optarg);
            if (!im->watermark) {
                rrd_set_error("cannot allocate memory for watermark");
                return;
            }
            break;
        case 'd':
        {
            if (im->daemon_addr != NULL)
            {
                rrd_set_error ("You cannot specify --daemon "
                        "more than once.");
                return;
            }
            im->daemon_addr = strdup(optarg);
            if (im->daemon_addr == NULL)
            {
              rrd_set_error("strdup failed");
              return;
            }
            break;
        }
        case '?':
            if (optopt != 0)
                rrd_set_error("unknown option '%c'", optopt);
            else
                rrd_set_error("unknown option '%s'", argv[optind - 1]);
            return;
        }
    }  
    pango_cairo_context_set_font_options(pango_layout_get_context(im->layout), im->font_options);
    pango_layout_context_changed(im->layout);
    if (im->logarithmic && im->minval <= 0) {
        rrd_set_error
            ("for a logarithmic yaxis you must specify a lower-limit > 0");
        return;
    }
    if (rrd_proc_start_end(&start_tv, &end_tv, &start_tmp, &end_tmp) == -1) {
        return;
    }
    if (start_tmp < 3600 * 24 * 365 * 10) {
        rrd_set_error
            ("the first entry to fetch should be after 1980 (%ld)",
             start_tmp);
        return;
    }
    if (end_tmp < start_tmp) {
        rrd_set_error
            ("start (%ld) should be less than end (%ld)", start_tmp, end_tmp);
        return;
    }
    im->start = start_tmp;
    im->end = end_tmp;
    im->step = max((long) im->step, (im->end - im->start) / im->xsize);
}