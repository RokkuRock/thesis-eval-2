set_lenIV(char *line)
{
  char *p = strstr(line, "/lenIV ");
  if (p && (isdigit(p[7]) || p[7] == '+' || p[7] == '-')) {
    lenIV = atoi(p + 7);
  }
}