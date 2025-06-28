int read_password(unsigned char* buffer, encryptmode_t mode)
{
#ifndef WIN32
#define PASS_EOF EOF
    struct termios t;                    
    int echo_enabled;                    
    int tty;                             
    FILE* ftty;                          
    unsigned char pwd[MAX_PASSWD_BUF];
    unsigned char pwd_confirm[MAX_PASSWD_BUF];
    unsigned char* p;                    
#else
#define PASS_EOF L'\x003'
    FILE* ftty = stderr;                 
    wchar_t* pwd = (wchar_t *)buffer;
    wchar_t pwd_confirm[MAX_PASSWD_LEN+1];
    wchar_t* p;                          
#endif
    int c;                               
    int chars_read;                      
    int i;                               
    int match;                           
#ifndef WIN32
    ftty = fopen("/dev/tty", "r+");
    if (ftty == NULL)
    {
        return AESCRYPT_READPWD_FOPEN;
    }
    tty = fileno(ftty);
    if (tty < 0)
    {
        return AESCRYPT_READPWD_FILENO;
    }
    if (tcgetattr(tty, &t) < 0)
    {
        fclose(ftty);
        return AESCRYPT_READPWD_TCGETATTR;
    }
#endif
    for (i = 0; (i == 0) || (i == 1 && mode == ENC); i++)
    {
        if (!i)
        {
            p = pwd;
        }
        else
        {
            p = pwd_confirm;
        }
        if (i)
        {
            fprintf(ftty, "Re-");
        }
        fprintf(ftty, "Enter password: ");
        fflush(ftty);
#ifndef WIN32
        if (t.c_lflag & ECHO)
        {
            t.c_lflag &= ~ECHO;
            if (tcsetattr(tty, TCSANOW, &t) < 0)
            {
                memset_secure(pwd, 0, MAX_PASSWD_BUF);
                memset_secure(pwd_confirm, 0, MAX_PASSWD_BUF);
                fclose(ftty);
                return AESCRYPT_READPWD_TCSETATTR;
            }
            echo_enabled = 1;
        }
        else
        {
            echo_enabled = 0;
        }
#endif
        chars_read = 0;
#ifdef WIN32
        while (((c = _getwch()) != L'\r') && (c != PASS_EOF))
#else
        while (((c = fgetc(ftty)) != '\n') && (c != PASS_EOF))
#endif
        {
            if (chars_read <= MAX_PASSWD_LEN)
            {
#ifdef WIN32
                p[chars_read] = (wchar_t) c;
#else
                p[chars_read] = (char) c;
#endif
            }
            chars_read++;
        }
        if (chars_read <= MAX_PASSWD_LEN)
        {
            p[chars_read] = '\0';
        }
        fprintf(ftty, "\n");
#ifndef WIN32
        if (echo_enabled)
        {
            t.c_lflag |= ECHO;
            if (tcsetattr(tty, TCSANOW, &t) < 0)
            {
                memset_secure(pwd, 0, MAX_PASSWD_BUF);
                memset_secure(pwd_confirm, 0, MAX_PASSWD_BUF);
                fclose(ftty);
                return AESCRYPT_READPWD_TCSETATTR;
            }
        }
#endif
        if (c == PASS_EOF)
        {
            memset_secure(pwd, 0, MAX_PASSWD_BUF);
            memset_secure(pwd_confirm, 0, MAX_PASSWD_BUF);
            if (ftty != stderr)
                fclose(ftty);
            return AESCRYPT_READPWD_FGETC;
        }
        if (chars_read > MAX_PASSWD_LEN)
        {
            memset_secure(pwd, 0, MAX_PASSWD_BUF);
            memset_secure(pwd_confirm, 0, MAX_PASSWD_BUF);
            if (ftty != stderr)
                fclose(ftty);
            return AESCRYPT_READPWD_TOOLONG;
        }
    }
    if (ftty != stderr)
        fclose(ftty);
    if (mode == ENC)
    {
        match = strcmp((char*)pwd, (char*)pwd_confirm);
        memset_secure(pwd_confirm, 0, MAX_PASSWD_BUF);
        if (match != 0)
        {
            memset_secure(pwd, 0, MAX_PASSWD_BUF);
            return AESCRYPT_READPWD_NOMATCH;
        }
    }
#ifdef WIN32
    chars_read *= 2;
#else
    chars_read = passwd_to_utf16(
       pwd,
       chars_read,
       MAX_PASSWD_LEN,
       buffer);
    if (chars_read < 0) {
        memset_secure(pwd_confirm, 0, MAX_PASSWD_BUF);
        memset_secure(pwd, 0, MAX_PASSWD_BUF);
        return AESCRYPT_READPWD_ICONV;
    }
#endif
    return chars_read;
}