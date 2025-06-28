Jsi_Interp* Jsi_Main(Jsi_InterpOpts *opts)
{
    int rc = 0;
    Jsi_Interp* interp = NULL;
    int argc = 0, first = 1;
    char **argv = NULL;
    if (opts) {
        interp = opts->interp;
        argc = opts->argc;
        argv = opts->argv;
    }
    if (!interp)
        interp = Jsi_InterpNew(opts);
    if (!interp)
        return NULL;
    Jsi_InterpOnDelete(interp, &jsi_InterpDelete, (void*)&jsi_InterpDelete);
    argc -= interp->iskips;
    argv += interp->iskips;
#ifndef NO_JAZ
    Jsi_Value *v = Jsi_Executable(interp);
    const char *exeFile = (v?Jsi_ValueString(interp, v, NULL):NULL);
    int jsFound = 0;
    if (v && (argc != 2 || Jsi_Strcmp(argv[1], "--nozvfs"))) {
        rc = Jsi_EvalZip(interp, exeFile, JSI_ZVFS_DIR, &jsFound);
        if (rc == JSI_OK) {
            interp->selfZvfs = 1;
            if (!jsFound) {
#if (JSI__FILESYS && JSI__ZVFS)
                fprintf(stderr, "warning: no main.jsi or autoload.jsi\n");
#endif
            }
            if (jsi_deleted)
                return jsi_DoExit(interp, jsi_exitCode);
            else if (rc != 0) {
                fprintf(stderr, "Error\n");
                return jsi_DoExit(interp, 1);
            }
        }
    }
#endif
    const char *ext = NULL, *ai1, *iext = (argc<=1?NULL:Jsi_Strrchr(argv[1], '.'));
    if (interp->selfZvfs && iext && Jsi_Strcmp(iext,".fossil")==0) {
        rc = Jsi_EvalString(interp, "runModule('Archive');", JSI_EVAL_ISMAIN);
        goto done;
    }
    Jsi_ShiftArgs(interp, NULL);
    if (argc <= 1) {
        if (interp->opts.no_interactive && !interp->interactive)
            return interp;
        rc = Jsi_Interactive(interp, JSI_OUTPUT_QUOTE|JSI_OUTPUT_NEWLINES);
        goto done;
    }
    ai1 = argv[1];
    if ((!Jsi_Strcmp(ai1, "-help") || !Jsi_Strcmp(ai1, "-h")) && argc<=3) {
        if (argc>2) {
            if (Jsi_PkgRequire(interp, "Help", 0)>=0) {
                char tbuf[BUFSIZ];
                snprintf(tbuf, sizeof(tbuf), "return runModule('Help', '%s'.trim().split(null));", argv[2]);
                Jsi_RC rc = Jsi_EvalString(interp, tbuf, 0);
                const char *hstr = Jsi_ValueToString(interp, interp->retValue, NULL);
                if (rc == JSI_OK)
                    puts(hstr);
                return jsi_DoExit(interp, 1);
            }
        }
        dohelp:
        puts("USAGE:\n  jsish [PREFIX-OPTS] [COMMAND-OPTS|FILE] ...\n"
          "\nPREFIX-OPTS:\n"
          "  --C FILE\tOption file of config options.\n"
          "  --F\t\tTrace all function calls and returns.\n"
          "  --I OPT:VAL\tInterp option: equivalent to Interp.conf({OPT:VAL}).\n"
          "  --L PATH\tSet safeMode to \"lockdown\" using PATH for safe(Read/Write)Dirs.\n"
          "  --T OPT\tTypecheck option: equivalent to \"use OPT\".\n"
          "  --U\t\tDisplay unittest output, minus pass/fail compare.\n"
          "  --V\t\tSame as --U, but adds file and line number to output.\n"
          "\nCOMMAND-OPTS:\n"
          "  -a\t\tArchive: mount an archive (zip, sqlar or fossil repo) and run module.\n"
          "  -c\t\tCData: generate .c or JSON output from a .jsc description.\n"
          "  -d\t\tDebug: console script debugger.\n"
          "  -e CODE ...\tEvaluate javascript CODE.\n"
          "  -g\t\tGendeep: generate html output from markdeep source.\n"
          "  -h ?CMD?\tHelp: show help for jsish or its commands.\n"
          "  -m\t\tModule: utility create/manage/invoke a Module.\n"
          "  -s\t\tSafe: runs script in safe sub-interp.\n"
          "  -u\t\tUnitTest: test script file(s) or directories .js/.jsi files.\n"
          "  -w\t\tWget: web client to download file from url.\n"
          "  -v\t\tVersion: show version detail: add an arg to show only X.Y.Z\n"
          "  -z\t\tZip: append/manage zip files at end of executable.\n"
          "  -D\t\tDebugUI: web-gui script debugger.\n"
          "  -J\t\tJSpp: preprocess javascript for web.\n"
          "  -S\t\tSqliteUI: web-gui for sqlite database file.\n"
          "  -W\t\tWebsrv: web server to serve out content.\n"
          "\nInterp options may also be set via the confFile.'\n"
           );
        return jsi_DoExit(interp, 1);
    }
    if (!Jsi_Strcmp(ai1, "-version"))
        ai1 = "-v";
    if (ai1[0] == '-') {
        switch (ai1[1]) {
            case 'a':
                rc = Jsi_EvalString(interp, "runModule('Archive');", JSI_EVAL_ISMAIN);
                break;
            case 'c':
                rc = Jsi_EvalString(interp, "runModule('Cdata');", JSI_EVAL_ISMAIN);
                break;
            case 'd':
                interp->debugOpts.isDebugger = 1;
                rc = Jsi_EvalString(interp, "runModule('Debug');", JSI_EVAL_ISMAIN);
                break;
            case 'D':
                interp->debugOpts.isDebugger = 1;
                rc = Jsi_EvalString(interp, "runModule('DebugUI');", JSI_EVAL_ISMAIN);
                break;
            case 'e':
                if (argc < 3)
                    rc = Jsi_LogError("missing argument");
                else {
                    rc = Jsi_EvalString(interp, argv[2], JSI_EVAL_ISMAIN|JSI_EVAL_NOSKIPBANG);
                    if (rc == JSI_OK && argc>3) {
                        first += 2;
                        Jsi_ShiftArgs(interp, NULL);
                        Jsi_ShiftArgs(interp, NULL);
                        goto dofile;
                    }
                }
                break;
            case 'g':
                rc = Jsi_EvalString(interp, "runModule('GenDeep');", JSI_EVAL_ISMAIN);
                break;
            case 'h':
                goto dohelp;
            case 'J':
                rc = Jsi_EvalString(interp, "runModule('Jspp');", JSI_EVAL_ISMAIN);
                break;
            case 'm':
                if (argc <= 2 || argv[2][0] == '-')
                    rc = Jsi_EvalString(interp, "runModule('Module');", JSI_EVAL_ISMAIN);
                else {
                    Jsi_DString dStr = {}, eStr = {};
                    const char *cps, *cpe;
                    cps = Jsi_Strrchr(argv[2], '/');
                    if (cps) cps++; else cps = argv[2];
                    cpe = Jsi_Strrchr(cps, '.');
                    int len = (cpe?cpe-cps:(int)Jsi_Strlen(cps));
                    if (cpe)
                        Jsi_DSPrintf(&dStr, "source(\"%s\");", argv[2]);
                    else
                        Jsi_DSPrintf(&dStr, "require(\"%s\");", argv[2]);
                    Jsi_DSPrintf(&dStr, "puts(runModule(\"%.*s\",console.args.slice(1)));", len, cps);
                    rc = Jsi_EvalString(interp, Jsi_DSValue(&dStr), JSI_EVAL_NOSKIPBANG);
                    Jsi_DSFree(&dStr);
                    Jsi_DSFree(&eStr);
                }
                break;
            case 's':
                rc = Jsi_EvalString(interp, "runModule('Safe');", JSI_EVAL_ISMAIN);
                break;
            case 'S':
                rc = Jsi_EvalString(interp, "runModule('SqliteUI');", JSI_EVAL_ISMAIN);
                break;
            case 'u':
                rc = Jsi_EvalString(interp, "exit(runModule('UnitTest'));", JSI_EVAL_ISMAIN);
                break;
            case 'v': {
                char str[200] = "\n";
                Jsi_Value* fval = Jsi_ValueNewStringKey(interp, "/zvfs/lib/sourceid.txt");
                if (!Jsi_Access(interp, fval, R_OK)) {
                    Jsi_Channel chan = Jsi_Open(interp, fval, "r");
                    if (chan)
                        Jsi_Read(interp, chan, str, sizeof(str));
                }
                if (argc>2)
                    printf("%u.%u.%u\n", JSI_VERSION_MAJOR, JSI_VERSION_MINOR, JSI_VERSION_RELEASE);
                else 
                    printf("%u.%u.%u %." JSI_VERFMT_LEN JSI_NUMGFMT " %s", JSI_VERSION_MAJOR, JSI_VERSION_MINOR, JSI_VERSION_RELEASE, Jsi_Version(), str);
                return jsi_DoExit(interp, 1);
            }
            case 'w':
                rc = Jsi_EvalString(interp, "runModule('Wget');", JSI_EVAL_ISMAIN);
                break;
            case 'W':
                rc = Jsi_EvalString(interp, "runModule('Websrv');", JSI_EVAL_ISMAIN);
                break;
            case 'z':
                rc = Jsi_EvalString(interp, "runModule('Zip');", JSI_EVAL_ISMAIN);
                break;
            default:
                puts("usage: jsish [  --C FILE | --I OPT:VAL | --L PATH | --T OPT | --U | --V | --F ] | -e STRING |\n\t"
                "| -a | -c | -d | -D | -h | -m | -s | -S | -u | -v | -w | -W | -z | FILE ...\nUse -help for long help.");
                return jsi_DoExit(interp, 1);
        }
    } else {
dofile:
        ext = Jsi_Strrchr(argv[first], '.');
        if (ext && (Jsi_Strcmp(ext,".zip")==0 ||Jsi_Strcmp(ext,".jsz")==0 ) ) {
            rc = Jsi_EvalZip(interp, argv[first], NULL, &jsFound);
            if (rc<0) {
                fprintf(stderr, "zip mount failed\n");
                return jsi_DoExit(interp, 1);
            }
            if (!(jsFound&JSI_ZIP_MAIN)) {
                fprintf(stderr, "main.jsi not found\n");
                return jsi_DoExit(interp, 1);
            }
        } else if (ext && !Jsi_Strcmp(ext,".jsc")) {
            Jsi_DString dStr = {};
            Jsi_DSPrintf(&dStr, "console.args.unshift('%s'); runModule('CData');", argv[first]);
            rc = Jsi_EvalString(interp, Jsi_DSValue(&dStr), JSI_EVAL_ISMAIN|JSI_EVAL_NOSKIPBANG);
            Jsi_DSFree(&dStr);
        } else {
            if (argc>1) {
                jsi_vf = Jsi_ValueNewStringKey(interp, argv[first]);
                Jsi_IncrRefCount(interp, jsi_vf);
            }
            rc = Jsi_EvalFile(interp, jsi_vf, JSI_EVAL_ARGV0|JSI_EVAL_AUTOINDEX|JSI_EVAL_ISMAIN);
            if (jsi_vf) {
                Jsi_DecrRefCount(interp, jsi_vf);
                jsi_vf = NULL;
            }
        }
    }
    if (jsi_deleted)  
        return jsi_DoExit(rc==JSI_EXIT?NULL:interp, jsi_exitCode);
    if (rc == 0) {
        Jsi_Value *ret = Jsi_ReturnValue(interp);
        if (!Jsi_ValueIsType(interp, ret, JSI_VT_UNDEF)) {
            Jsi_DString dStr = {};
            fputs(Jsi_ValueGetDString(interp, ret, &dStr, 0), stdout);
            Jsi_DSFree(&dStr);
            fputs("\n", stdout);
        }
    } else {
        if (!interp->parent && !interp->isHelp)
            fputs("ERROR\n", stderr);
        return jsi_DoExit(interp, 1);
    }
done:
    if (rc == JSI_EXIT) {
        if (opts)
            opts->exitCode = jsi_exitCode;
        return NULL;
    }
    if (jsi_deleted == 0 && interp->opts.auto_delete) {
        Jsi_InterpDelete(interp);
        return NULL;
    }
    return interp;
}