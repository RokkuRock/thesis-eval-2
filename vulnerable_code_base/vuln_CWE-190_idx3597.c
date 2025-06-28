int gdAlphaBlend (int dst, int src) {
    int src_alpha = gdTrueColorGetAlpha(src);
    int dst_alpha, alpha, red, green, blue;
    int src_weight, dst_weight, tot_weight;
    if( src_alpha == gdAlphaOpaque )
        return src;
    dst_alpha = gdTrueColorGetAlpha(dst);
    if( src_alpha == gdAlphaTransparent )
        return dst;
    if( dst_alpha == gdAlphaTransparent )
        return src;
    src_weight = gdAlphaTransparent - src_alpha;
    dst_weight = (gdAlphaTransparent - dst_alpha) * src_alpha / gdAlphaMax;
    tot_weight = src_weight + dst_weight;
    alpha = src_alpha * dst_alpha / gdAlphaMax;
    red = (gdTrueColorGetRed(src) * src_weight
           + gdTrueColorGetRed(dst) * dst_weight) / tot_weight;
    green = (gdTrueColorGetGreen(src) * src_weight
           + gdTrueColorGetGreen(dst) * dst_weight) / tot_weight;
    blue = (gdTrueColorGetBlue(src) * src_weight
           + gdTrueColorGetBlue(dst) * dst_weight) / tot_weight;
    return ((alpha << 24) + (red << 16) + (green << 8) + blue);
}