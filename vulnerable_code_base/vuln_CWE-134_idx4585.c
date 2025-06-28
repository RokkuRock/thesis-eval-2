int bad_format(
    char *fmt)
{
    char     *ptr;
    int       n = 0;
    ptr = fmt;
    while (*ptr != '\0')
        if (*ptr++ == '%') {
            if (*ptr == '\0')
                return 1;
            if (*ptr == 's' || *ptr == 'S' || *ptr == '%')
                ptr++;
            else if (*ptr == 'c') {
                ptr++;
                n = 1;
            }
            else {
                if (*ptr == ' ' || *ptr == '+' || *ptr == '-')
                    ptr++;
                while (*ptr >= '0' && *ptr <= '9')
                    ptr++;
                if (*ptr == '.')
                    ptr++;
                while (*ptr >= '0' && *ptr <= '9')
                    ptr++;
                if (*ptr++ != 'l')
                    return 1;
                if (*ptr == 'e' || *ptr == 'f' || *ptr == 'g')
                    ptr++;
                else
                    return 1;
                n++;
            }
        }
    return (n != 1);
}