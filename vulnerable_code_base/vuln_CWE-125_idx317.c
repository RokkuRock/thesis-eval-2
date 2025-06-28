static u_int mp_dss_len(const  struct mp_dss *m, int csum)
{
        u_int len;
        len = 4;
        if (m->flags & MP_DSS_A) {
                len += (m->flags & MP_DSS_a) ? 8 : 4;
        }
        if (m->flags & MP_DSS_M) {
                len += (m->flags & MP_DSS_m) ? 14 : 10;
                if (csum)
                        len += 2;
	}
	return len;
}