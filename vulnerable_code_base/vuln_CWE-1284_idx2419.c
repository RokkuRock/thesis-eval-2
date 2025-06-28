void usage(const char *progname)
{
    const char* progname_real;  
    progname_real = strrchr(progname, '/');
    if (progname_real == NULL)  
    {
        progname_real = progname;
    }
    else
    {
        progname_real++;
    }
    fprintf(stderr, "\nusage: %s {-e|-d} [ { -p <password> | -k <keyfile> } ] { [-o <output filename>] <file> | <file> [<file> ...] }\n\n",
            progname_real);
}