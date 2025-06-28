static char *get_header(FILE *fp)
{
    long start;
    char *header;
    header = calloc(1, 1024);
    start = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    SAFE_E(fread(header, 1, 1023, fp), 1023, "Failed to load PDF header.\n");
    fseek(fp, start, SEEK_SET);
    return header;
}