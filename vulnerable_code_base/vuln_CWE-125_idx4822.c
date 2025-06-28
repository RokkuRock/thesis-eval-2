read_pbm_integer(j_compress_ptr cinfo, FILE *infile, unsigned int maxval)
{
  register int ch;
  register unsigned int val;
  do {
    ch = pbm_getc(infile);
    if (ch == EOF)
      ERREXIT(cinfo, JERR_INPUT_EOF);
  } while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r');
  if (ch < '0' || ch > '9')
    ERREXIT(cinfo, JERR_PPM_NONNUMERIC);
  val = ch - '0';
  while ((ch = pbm_getc(infile)) >= '0' && ch <= '9') {
    val *= 10;
    val += ch - '0';
  }
  if (val > maxval)
    ERREXIT(cinfo, JERR_PPM_TOOLARGE);
  return val;
}