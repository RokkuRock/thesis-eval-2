jsi_wsGetHeaders(jsi_wsPss *pss, struct lws *wsi, Jsi_DString* dStr, int lens[], int hmax)
{
    int n = 0, i = 0, nlen;
    char buf[1000];
    const char *cp;
    while ((cp = (char*)lws_token_to_string((enum lws_token_indexes)n))) {
        int len = lws_hdr_copy(wsi, buf, sizeof(buf), ( enum lws_token_indexes)n);
        n++;
        if (i>=(n*2+2)) break;
        if (len<=0) continue;
        buf[sizeof(buf)-1] = 0;
        if (!buf[0]) continue;
        nlen = Jsi_Strlen(cp);
        if (nlen>0 && cp[nlen-1]==' ') nlen--;
        if (nlen>0 && cp[nlen-1]==':') nlen--;
        Jsi_DSAppendLen(dStr, cp, nlen);
        Jsi_DSAppend(dStr, "=", buf, "\n", NULL);
        if (lens) {
            lens[i++] = nlen;
            lens[i++] = Jsi_Strlen(buf);
        }
    }
    return i;
}