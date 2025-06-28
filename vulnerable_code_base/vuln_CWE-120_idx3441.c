static Jsi_RC NumberToStringCmd(Jsi_Interp *interp, Jsi_Value *args, Jsi_Value *_this,
    Jsi_Value **ret, Jsi_Func *funcPtr)
{
    char buf[500];
    int radix = 10, skip = 0, argc = Jsi_ValueGetLength(interp, args);
    Jsi_Number num;
    Jsi_Value *v;
    ChkStringN(_this, funcPtr, v);
    Jsi_GetDoubleFromValue(interp, v, &num);
    if (argc>skip && (Jsi_GetIntFromValue(interp, Jsi_ValueArrayIndex(interp, args, skip), &radix) != JSI_OK
        || radix<2))
        return JSI_ERROR;
    if (argc==skip)
        return jsi_ObjectToStringCmd(interp, args, _this, ret, funcPtr);
    switch (radix) {
        case 16: snprintf(buf, sizeof(buf), "%" PRIx64, (Jsi_Wide)num); break;
        case 8: snprintf(buf, sizeof(buf), "%" PRIo64, (Jsi_Wide)num); break;
        case 10: snprintf(buf, sizeof(buf), "%" PRId64, (Jsi_Wide)num); break;
        default: return jsi_ObjectToStringCmd(interp, args, _this, ret, funcPtr);
    }
    Jsi_ValueMakeStringDup(interp, ret, buf);
    return JSI_OK;
}