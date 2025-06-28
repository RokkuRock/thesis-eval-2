char *format_timespan(char *buf, size_t l, usec_t t, usec_t accuracy) {
        static const struct {
                const char *suffix;
                usec_t usec;
        } table[] = {
                { "y",     USEC_PER_YEAR   },
                { "month", USEC_PER_MONTH  },
                { "w",     USEC_PER_WEEK   },
                { "d",     USEC_PER_DAY    },
                { "h",     USEC_PER_HOUR   },
                { "min",   USEC_PER_MINUTE },
                { "s",     USEC_PER_SEC    },
                { "ms",    USEC_PER_MSEC   },
                { "us",    1               },
        };
        char *p = buf;
        bool something = false;
        assert(buf);
        assert(l > 0);
        if (t == USEC_INFINITY) {
                strncpy(p, "infinity", l-1);
                p[l-1] = 0;
                return p;
        }
        if (t <= 0) {
                strncpy(p, "0", l-1);
                p[l-1] = 0;
                return p;
        }
        for (size_t i = 0; i < ELEMENTSOF(table); i++) {
                int k = 0;
                size_t n;
                bool done = false;
                usec_t a, b;
                if (t <= 0)
                        break;
                if (t < accuracy && something)
                        break;
                if (t < table[i].usec)
                        continue;
                if (l <= 1)
                        break;
                a = t / table[i].usec;
                b = t % table[i].usec;
                if (t < USEC_PER_MINUTE && b > 0) {
                        signed char j = 0;
                        for (usec_t cc = table[i].usec; cc > 1; cc /= 10)
                                j++;
                        for (usec_t cc = accuracy; cc > 1; cc /= 10) {
                                b /= 10;
                                j--;
                        }
                        if (j > 0) {
                                k = snprintf(p, l,
                                             "%s"USEC_FMT".%0*"PRI_USEC"%s",
                                             p > buf ? " " : "",
                                             a,
                                             j,
                                             b,
                                             table[i].suffix);
                                t = 0;
                                done = true;
                        }
                }
                if (!done) {
                        k = snprintf(p, l,
                                     "%s"USEC_FMT"%s",
                                     p > buf ? " " : "",
                                     a,
                                     table[i].suffix);
                        t = b;
                }
                n = MIN((size_t) k, l);
                l -= n;
                p += n;
                something = true;
        }
        *p = 0;
        return buf;
}