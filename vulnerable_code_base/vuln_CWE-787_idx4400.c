int main(int argc, char *argv[])
{
    int result;
    int infoOnly = 0;
    int writeToStdio = 0;
    int readFromStdin = 0;
    int object_type = LC;
    int def_srate = 0;
    int downMatrix = 0;
    int format = 1;
    int outputFormat = FAAD_FMT_16BIT;
    int outfile_set = 0;
    int adts_out = 0;
    int old_format = 0;
    int showHelp = 0;
    int mp4file = 0;
    int noGapless = 0;
    char *fnp;
    char aacFileName[255];
    char audioFileName[255];
    char adtsFileName[255];
    unsigned char header[8];
    float length = 0;
    FILE *hMP4File;
#ifdef _WIN32
    long begin;
#else
    clock_t begin;
#endif
    unsigned long cap = NeAACDecGetCapabilities();
    progName = argv[0];
    while (1) {
        int c = -1;
        int option_index = 0;
        static struct option long_options[] = {
            { "quiet",      0, 0, 'q' },
            { "outfile",    0, 0, 'o' },
            { "adtsout",    0, 0, 'a' },
            { "oldformat",  0, 0, 't' },
            { "format",     0, 0, 'f' },
            { "bits",       0, 0, 'b' },
            { "samplerate", 0, 0, 's' },
            { "objecttype", 0, 0, 'l' },
            { "downmix",    0, 0, 'd' },
            { "info",       0, 0, 'i' },
            { "stdio",      0, 0, 'w' },
            { "stdio",      0, 0, 'g' },
            { "help",       0, 0, 'h' },
            { 0, 0, 0, 0 }
        };
        c = getopt_long(argc, argv, "o:a:s:f:b:l:wgdhitq",
            long_options, &option_index);
        if (c == -1)
            break;
        switch (c) {
        case 'o':
            if (optarg)
            {
                outfile_set = 1;
                strcpy(audioFileName, optarg);
            }
            break;
        case 'a':
            if (optarg)
            {
                adts_out = 1;
                strcpy(adtsFileName, optarg);
            }
            break;
        case 's':
            if (optarg)
            {
                char dr[10];
                if (sscanf(optarg, "%s", dr) < 1) {
                    def_srate = 0;
                } else {
                    def_srate = atoi(dr);
                }
            }
            break;
        case 'f':
            if (optarg)
            {
                char dr[10];
                if (sscanf(optarg, "%s", dr) < 1)
                {
                    format = 1;
                } else {
                    format = atoi(dr);
                    if ((format < 1) || (format > 2))
                        showHelp = 1;
                }
            }
            break;
        case 'b':
            if (optarg)
            {
                char dr[10];
                if (sscanf(optarg, "%s", dr) < 1)
                {
                    outputFormat = FAAD_FMT_16BIT;  
                } else {
                    outputFormat = atoi(dr);
                    if ((outputFormat < 1) || (outputFormat > 5))
                        showHelp = 1;
                }
            }
            break;
        case 'l':
            if (optarg)
            {
                char dr[10];
                if (sscanf(optarg, "%s", dr) < 1)
                {
                    object_type = LC;  
                } else {
                    object_type = atoi(dr);
                    if ((object_type != LC) &&
                        (object_type != MAIN) &&
                        (object_type != LTP) &&
                        (object_type != LD))
                    {
                        showHelp = 1;
                    }
                }
            }
            break;
        case 't':
            old_format = 1;
            break;
        case 'd':
            downMatrix = 1;
            break;
        case 'w':
            writeToStdio = 1;
            break;
        case 'g':
            noGapless = 1;
            break;
        case 'i':
            infoOnly = 1;
            break;
        case 'h':
            showHelp = 1;
            break;
        case 'q':
            quiet = 1;
            break;
        default:
            break;
        }
    }
    faad_fprintf(stderr, " *********** Ahead Software MPEG-4 AAC Decoder V%s ******************\n\n", FAAD2_VERSION);
    faad_fprintf(stderr, " Build: %s\n", __DATE__);
    faad_fprintf(stderr, " Copyright 2002-2004: Ahead Software AG\n");
    faad_fprintf(stderr, " http://www.audiocoding.com\n");
    if (cap & FIXED_POINT_CAP)
        faad_fprintf(stderr, " Fixed point version\n");
    else
        faad_fprintf(stderr, " Floating point version\n");
    faad_fprintf(stderr, "\n");
    faad_fprintf(stderr, " This program is free software; you can redistribute it and/or modify\n");
    faad_fprintf(stderr, " it under the terms of the GNU General Public License.\n");
    faad_fprintf(stderr, "\n");
    faad_fprintf(stderr, " **************************************************************************\n\n");
    if (((argc - optind) < 1) || showHelp)
    {
        usage();
        return 1;
    }
#if 0
    if (writeToStdio == 1)
    {
        format = 2;
    }
#endif
    strcpy(aacFileName, argv[optind]);
#ifdef _WIN32
    begin = GetTickCount();
#else
    begin = clock();
#endif
    if(!writeToStdio && !outfile_set)
    {
        strcpy(audioFileName, aacFileName);
        fnp = (char *)strrchr(audioFileName,'.');
        if (fnp)
            fnp[0] = '\0';
        strcat(audioFileName, file_ext[format]);
    }
    if (0 == strcmp(aacFileName, "-")) {
   	faad_fprintf(stderr, "Reading from stdin: %s\n", aacFileName);
	readFromStdin = 1;
	hMP4File  = stdin;
#ifdef _WIN32
        setmode(fileno(stdin), O_BINARY);
#endif
    } else {
    	mp4file = 0;
    	hMP4File = fopen(aacFileName, "rb");
    	if (!hMP4File)
    	{
    	    faad_fprintf(stderr, "Error opening file: %s\n", aacFileName);
    	    return 1;
    	}
    }
    fread(header, 1, 8, hMP4File);
    if (! readFromStdin )
      fclose(hMP4File);
    if (header[4] == 'f' && header[5] == 't' && header[6] == 'y' && header[7] == 'p')
        mp4file = 1;
    if (mp4file)
    {
        result = decodeMP4file(aacFileName, audioFileName, adtsFileName, writeToStdio,
            outputFormat, format, downMatrix, noGapless, infoOnly, adts_out, &length);
    } else {
	if (readFromStdin == 1) {
		ungetc(header[7],hMP4File);
		ungetc(header[6],hMP4File);
		ungetc(header[5],hMP4File);
		ungetc(header[4],hMP4File);
		ungetc(header[3],hMP4File);
		ungetc(header[2],hMP4File);
		ungetc(header[1],hMP4File);
		ungetc(header[0],hMP4File);
	}
        result = decodeAACfile(aacFileName, audioFileName, adtsFileName, writeToStdio,
            def_srate, object_type, outputFormat, format, downMatrix, infoOnly, adts_out,
            old_format, &length);
    }
    if (!result && !infoOnly)
    {
#ifdef _WIN32
        float dec_length = (float)(GetTickCount()-begin)/1000.0;
        SetConsoleTitle("FAAD");
#else
        float dec_length = (float)(clock() - begin)/(float)CLOCKS_PER_SEC;
#endif
        faad_fprintf(stderr, "Decoding %s took: %5.2f sec. %5.2fx real-time.\n", aacFileName,
            dec_length, length/dec_length);
    }
    return 0;
}