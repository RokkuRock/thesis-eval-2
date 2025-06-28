format_number(int  n,		 
              char f)		 
{
  static const char *ones[10] =	 
		{
		  "",	"i",	"ii",	"iii",	"iv",
		  "v",	"vi",	"vii",	"viii",	"ix"
		},
		*tens[10] =	 
		{
		  "",	"x",	"xx",	"xxx",	"xl",
		  "l",	"lx",	"lxx",	"lxxx",	"xc"
		},
		*hundreds[10] =	 
		{
		  "",	"c",	"cc",	"ccc",	"cd",
		  "d",	"dc",	"dcc",	"dccc",	"cm"
		};
  static const char *ONES[10] =	 
		{
		  "",	"I",	"II",	"III",	"IV",
		  "V",	"VI",	"VII",	"VIII",	"IX"
		},
		*TENS[10] =	 
		{
		  "",	"X",	"XX",	"XXX",	"XL",
		  "L",	"LX",	"LXX",	"LXXX",	"XC"
		},
		*HUNDREDS[10] =	 
		{
		  "",	"C",	"CC",	"CCC",	"CD",
		  "D",	"DC",	"DCC",	"DCCC",	"CM"
		};
  static char	buffer[1024];	 
  switch (f)
  {
    default :
        buffer[0] = '\0';
	break;
    case 'a' :
        if (n >= (26 * 26))
	  buffer[0] = '\0';
        else if (n > 26)
          snprintf(buffer, sizeof(buffer), "%c%c", 'a' + (n / 26) - 1, 'a' + (n % 26) - 1);
        else
          snprintf(buffer, sizeof(buffer), "%c", 'a' + n - 1);
        break;
    case 'A' :
        if (n >= (26 * 26))
	  buffer[0] = '\0';
        else if (n > 26)
          snprintf(buffer, sizeof(buffer), "%c%c", 'A' + (n / 26) - 1, 'A' + (n % 26) - 1);
        else
          snprintf(buffer, sizeof(buffer), "%c", 'A' + n - 1);
        break;
    case '1' :
        snprintf(buffer, sizeof(buffer), "%d", n);
        break;
    case 'i' :
        if (n >= 1000)
	  buffer[0] = '\0';
	else
          snprintf(buffer, sizeof(buffer), "%s%s%s", hundreds[n / 100], tens[(n / 10) % 10], ones[n % 10]);
        break;
    case 'I' :
        if (n >= 1000)
	  buffer[0] = '\0';
	else
          snprintf(buffer, sizeof(buffer), "%s%s%s", HUNDREDS[n / 100], TENS[(n / 10) % 10], ONES[n % 10]);
        break;
  }
  return (buffer);
}