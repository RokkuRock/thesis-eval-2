indenterror(struct tok_state *tok)
{
    if (tok->alterror) {
        tok->done = E_TABSPACE;
        tok->cur = tok->inp;
        return 1;
    }
    if (tok->altwarning) {
#ifdef PGEN
        PySys_WriteStderr("inconsistent use of tabs and spaces "
                          "in indentation\n");
#else
        PySys_FormatStderr("%U: inconsistent use of tabs and spaces "
                          "in indentation\n", tok->filename);
#endif
        tok->altwarning = 0;
    }
    return 0;
}