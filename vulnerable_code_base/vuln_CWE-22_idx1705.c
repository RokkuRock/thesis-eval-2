static char *create_output_name(unsigned char *fname, unsigned char *dir,
			 int lower, int isunix, int utf8)
{
  unsigned char *p, *name, c, *fe, sep, slash;
  unsigned int x;
  sep   = (isunix) ? '/'  : '\\';  
  slash = (isunix) ? '\\' : '/';   
  x = strlen((char *) fname);
  if (utf8) x *= 3;
  if (dir) x += strlen((char *) dir);
  if (!(name = (unsigned char *) malloc(x + 2))) {
    fprintf(stderr, "out of memory!\n");
    return NULL;
  }
  *name = '\0';
  if (dir) {
    strcpy((char *) name, (char *) dir);
    strcat((char *) name, "/");
  }
  while (*fname == sep) fname++;
  p = &name[strlen((char *)name)];
  fe = &fname[strlen((char *)fname)];
  if (utf8) {
    do {
      if (fname >= fe) {
	free(name);
	return NULL;
      }
      if ((c = *fname++) < 0x80) x = c;
      else {
	if ((c >= 0xC0) && (c < 0xE0)) {
	  x = (c & 0x1F) << 6;
	  x |= *fname++ & 0x3F;
	}
	else if ((c >= 0xE0) && (c < 0xF0)) {
	  x = (c & 0xF) << 12;
	  x |= (*fname++ & 0x3F) << 6;
	  x |= *fname++ & 0x3F;
	}
	else x = '?';
      }
      if      (x == sep)   x = '/';
      else if (x == slash) x = '\\';
      else if (lower)      x = (unsigned int) tolower((int) x);
      if (x < 0x80) {
	*p++ = (unsigned char) x;
      }
      else if (x < 0x800) {
	*p++ = 0xC0 | (x >> 6);   
	*p++ = 0x80 | (x & 0x3F);
      }
      else {
	*p++ = 0xE0 | (x >> 12);
	*p++ = 0x80 | ((x >> 6) & 0x3F);
	*p++ = 0x80 | (x & 0x3F);
      }
    } while (x);
  }
  else {
    do {
      c = *fname++;
      if      (c == sep)   c = '/';
      else if (c == slash) c = '\\';
      else if (lower)      c = (unsigned char) tolower((int) c);
    } while ((*p++ = c));
  }
  return (char *) name;
}