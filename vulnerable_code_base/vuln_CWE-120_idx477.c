static void jsonNewDString(Jsi_Interp *interp, Jsi_DString *dStr, const char* str, int len)
{
    char buf[100], *dp = buf;
    const char *cp = str;
    int ulen;
    while ((cp-str)<len) {
        if (*cp == '\\') {
            switch (cp[1]) {
                case 'b': *dp++ = '\b'; break;
                case 'n': *dp++ = '\n'; break;
                case 'r': *dp++ = '\r'; break;
                case 'f': *dp++ = '\f'; break;
                case 't': *dp++ = '\t'; break;
                case '\"': *dp++ = '\"'; break;
                case '\\': *dp++ = '\\'; break;
                case 'u': 
                    if ((ulen=Jsi_UtfDecode(cp+2, dp))) {
                        dp += ulen;
                        cp += 4;
                    } else {                    
                        *dp++ = '\\';
                        *dp++ = 'u';
                    }
                    break;
            }
            cp+=2;
        } else {
            *dp++ = *cp++;
        }
        if ((dp-buf)>90) {
            *dp = 0;
            Jsi_DSAppendLen(dStr, buf, dp-buf);
            dp = buf;
        }
    }
    *dp = 0;
    Jsi_DSAppendLen(dStr, buf, dp-buf);
}