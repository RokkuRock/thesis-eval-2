static void mrled(u8 * RESTRICT in, u8 * RESTRICT out, s32 outlen) {
    s32 op = 0, ip = 0;
    s32 c, pc = -1;
    s32 t[256] = { 0 };
    s32 run = 0;
    for (s32 i = 0; i < 32; ++i) {
        c = in[ip++];
        for (s32 j = 0; j < 8; ++j) t[i * 8 + j] = (c >> j) & 1;
    }
    while (op < outlen) {
        c = in[ip++];
        if (t[c]) {
            for (run = 0; (pc = in[ip++]) == 255; run += 255)
                ;
            run += pc + 1;
            for (; run > 0 && op < outlen; --run) out[op++] = c;
        } else
            out[op++] = c;
    }
}