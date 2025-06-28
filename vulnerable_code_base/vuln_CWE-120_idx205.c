static Jsi_Value *jsi_hashFmtKey(Jsi_MapEntry* h, struct Jsi_MapOpts *opts, int flags)
{
    Jsi_HashEntry* hPtr = (Jsi_HashEntry*)h;
    void *key = Jsi_HashKeyGet(hPtr);
    if (opts->keyType == JSI_KEYS_ONEWORD)
        return Jsi_ValueNewNumber(opts->interp, (Jsi_Number)(intptr_t)key);
    char nbuf[100];
    snprintf(nbuf, sizeof(nbuf), "%p", key);
    return Jsi_ValueNewStringDup(opts->interp, nbuf);
}