int bad_format_imginfo(
    char *fmt)
{
    char     *ptr;
    int       n = 0;
    ptr = fmt;
    while (*ptr != '\0')
        if (*ptr++ == '%') {
            if (*ptr == '\0')
                return 1;
            if (*ptr == '%')
                ptr++;
            else if (*ptr == 's' || *ptr == 'S') {
                n = 1;
                ptr++;
            }
            else {
                if (*ptr == ' ')
                    ptr++;
                while (*ptr >= '0' && *ptr <= '9')
                    ptr++;
                if (*ptr++ != 'l')
                    return 1;
                if (*ptr == 'u')
                    ptr++;
                else
                    return 1;
                n++;
            }
        }
    return (n != 3);
}