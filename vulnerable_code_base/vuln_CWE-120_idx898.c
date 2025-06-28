void jsi_code_decode(Jsi_Interp *interp, jsi_OpCode *op, int currentip, char *buf, int bsiz)
{
    if (_JSICASTINT(op->op) < 0 || op->op >= OP_LASTOP) {
        snprintf(buf, bsiz, "Bad opcode[%d] at %d", op->op, currentip);
    }
    char nbuf[100];
    snprintf(nbuf, sizeof(nbuf), "%d#%d", currentip, op->Line);
    snprintf(buf, bsiz, "%-8s %s ", nbuf, jsi_op_names[op->op]);
    int sl = Jsi_Strlen(buf);
    char *bp = buf + sl;
    bsiz -= sl;
    if (op->op == OP_PUSHBOO || op->op == OP_FCALL || op->op == OP_EVAL ||
        op->op == OP_POP || op->op == OP_ASSIGN ||
        op->op == OP_RET || op->op == OP_NEWFCALL ||
        op->op == OP_DELETE || op->op == OP_CHTHIS ||
        op->op == OP_OBJECT || op->op == OP_ARRAY ||
        op->op == OP_SHF ||
        op->op == OP_INC || op->op == OP_DEC) snprintf(bp, bsiz, "%" PRId64, (Jsi_Wide)(uintptr_t)op->data);
    else if (op->op == OP_PUSHNUM) Jsi_NumberDtoA(interp, *((Jsi_Number *)op->data), bp, bsiz, 0);
    else if (op->op == OP_PUSHVSTR) {
        Jsi_String *ss = (Jsi_String*)op->data;
        snprintf(bp, bsiz, "\"%s\"", ss->str);
    } else if (op->op == OP_PUSHSTR || op->op == OP_LOCAL ||
             op->op == OP_SCATCH) snprintf(bp, bsiz, "\"%s\"", op->data ? (char*)op->data:"(NoCatch)");
    else if (op->op == OP_PUSHVAR) snprintf(bp, bsiz, "var: \"%s\"", ((jsi_FastVar *)op->data)->varname);
    else if (op->op == OP_PUSHFUN) snprintf(bp, bsiz, "func: 0x%" PRIx64, (Jsi_Wide)(uintptr_t)op->data);
    else if (op->op == OP_JTRUE || op->op == OP_JFALSE ||
             op->op == OP_JTRUE_NP || op->op == OP_JFALSE_NP ||
             op->op == OP_JMP) snprintf(bp, bsiz, "{%" PRIu64 "}\t#%" PRIu64 "", (Jsi_Wide)(uintptr_t)op->data, (Jsi_Wide)((uintptr_t)currentip + (uintptr_t)op->data));
    else if (op->op == OP_JMPPOP) {
        jsi_JmpPopInfo *jp = (jsi_JmpPopInfo*)op->data;
        snprintf(bp, bsiz, "{%d},%d\t#%d", jp->off, jp->topop, currentip + jp->off);
    }
    else if (op->op == OP_STRY) {
        jsi_TryInfo *t = (jsi_TryInfo *)op->data;
        snprintf(bp, bsiz, "{try:%d, catch:%d, final:%d}", t->trylen, t->catchlen, t->finallen);
    }
}