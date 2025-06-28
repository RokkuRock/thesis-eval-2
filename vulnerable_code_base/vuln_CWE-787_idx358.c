WRITE_JSON_ELEMENT(ArrStart) {
    ctx->commaNeeded[++ctx->depth] = false;
    return writeChar(ctx, '[');
}