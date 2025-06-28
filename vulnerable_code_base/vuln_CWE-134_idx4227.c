void hexdump(msg_info msg_info, const char *mem, unsigned int len)
{
    unsigned int i, j;
    char str[10 + HEXDUMP_COLS * 4 + 2];
    int c = 0;  
    for(i = 0; i < len + ((len % HEXDUMP_COLS) ? (HEXDUMP_COLS - len % HEXDUMP_COLS) : 0); i++)
    {
        if(i % HEXDUMP_COLS == 0)
            c += sprintf(&str[c], "0x%06x: ", i);
        if(i < len)
            c += sprintf(&str[c], "%02x ", 0xFF & mem[i]);
        else  
            c+= sprintf(&str[c], "   ");
        if(i % HEXDUMP_COLS == (HEXDUMP_COLS - 1)) {
            for(j = i - (HEXDUMP_COLS - 1); j <= i; j++) {
                if(j >= len)  
                    str[c++] = ' ';
                else if(isprint(mem[j]))  
                    str[c++] = 0xFF & mem[j];
                else  
                    str[c++] = '.';
            }
            str[c++] = '\n';
            str[c++] = 0;
            print_message(msg_info, str);
            c = 0;
        }
    }
}