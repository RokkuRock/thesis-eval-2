ctcompare(const char *a,		 
          const char *b)		 
{
  int	result = 0;			 
  while (*a && *b)
  {
    result |= *a ^ *b;
    a ++;
    b ++;
  }
  return (result);
}