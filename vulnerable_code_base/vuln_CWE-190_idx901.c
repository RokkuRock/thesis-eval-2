static int getnum (const char **fmt, int df) {
  if (!isdigit(**fmt))   
    return df;   
  else {
    int a = 0;
    do {
      a = a*10 + *((*fmt)++) - '0';
    } while (isdigit(**fmt));
    return a;
  }
}