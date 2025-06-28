int passwd_to_utf16(unsigned char *in_passwd,
                    int length,
                    int max_length,
                    unsigned char *out_passwd)
{
#ifdef WIN32
    int ret;
    (void)length;
    ret = MultiByteToWideChar(
        CP_ACP,
        0,
        (LPCSTR)in_passwd,
        -1,
        (LPWSTR)out_passwd,
        max_length / 2
    );
    if (ret == 0)
        return AESCRYPT_READPWD_ICONV;
    return ret * 2;
#else
#ifndef ENABLE_ICONV
    int i;
    for (i=0;i<length+1;i++) {
        out_passwd[i*2] = in_passwd[i];
        out_passwd[i*2+1] = 0;
    }
    return length*2;
#else
    unsigned char *ic_outbuf,
                  *ic_inbuf;
    iconv_t condesc;
    size_t ic_inbytesleft,
           ic_outbytesleft;
    max_length *= 2;
    ic_inbuf = in_passwd;
    ic_inbytesleft = length;
    ic_outbytesleft = max_length;
    ic_outbuf = out_passwd;
    setlocale(LC_CTYPE,"");
    if ((condesc = iconv_open("UTF-16LE", nl_langinfo(CODESET))) ==
        (iconv_t)(-1))
    {
        perror("Error in iconv_open");
        return -1;
    }
    if (iconv(condesc,
              (char ** const) &ic_inbuf,
              &ic_inbytesleft,
              (char ** const) &ic_outbuf,
              &ic_outbytesleft) == (size_t) -1)
    {
        switch (errno)
        {
            case E2BIG:
                fprintf(stderr, "Error: password too long\n");
                iconv_close(condesc);
                return -1;
                break;
            default:
                perror("Password conversion error");
                iconv_close(condesc);
                return -1;
        }
    }
    iconv_close(condesc);
    return (max_length - ic_outbytesleft);
#endif
#endif
}