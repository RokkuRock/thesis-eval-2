static Jsi_Value *jsi_treeFmtKey(Jsi_MapEntry* h, struct Jsi_MapOpts *opts, int flags)
{
    Jsi_TreeEntry* hPtr = (Jsi_TreeEntry*)h;
    void *key = Jsi_TreeKeyGet(hPtr);
    if (opts->keyType == JSI_KEYS_ONEWORD)
        return Jsi_ValueNewNumber(opts->interp, (Jsi_Number)(intptr_t)key);
    char nbuf[100];
    snprintf(nbuf, sizeof(nbuf), "%p", key);
    return Jsi_ValueNewStringDup(opts->interp, nbuf);
}