bittok2str_internal(register const struct tok *lp, register const char *fmt,
	   register u_int v, const char *sep)
{
        static char buf[256];  
        int buflen=0;
        register u_int rotbit;  
        register u_int tokval;
        const char * sepstr = "";
	while (lp != NULL && lp->s != NULL) {
            tokval=lp->v;    
            rotbit=1;
            while (rotbit != 0) {
		if (tokval == (v&rotbit)) {
                    buflen+=snprintf(buf+buflen, sizeof(buf)-buflen, "%s%s",
                                     sepstr, lp->s);
                    sepstr = sep;
                    break;
                }
                rotbit=rotbit<<1;  
            }
            lp++;
	}
        if (buflen == 0)
            (void)snprintf(buf, sizeof(buf), fmt == NULL ? "#%08x" : fmt, v);
        return (buf);
}