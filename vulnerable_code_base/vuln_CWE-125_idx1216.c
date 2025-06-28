gif_read_lzw(FILE *fp,			 
	     int  first_time,		 
 	     int  input_code_size)	 
{
  int		i,			 
		code,			 
		incode;			 
  static short	fresh = 0,		 
		code_size = 0,		 
		set_code_size = 0,	 
		max_code = 0,		 
		max_code_size = 0,	 
		firstcode = 0,		 
		oldcode = 0,		 
		clear_code = 0,		 
		end_code = 0,		 
		table[2][4096],		 
		stack[8192],		 
		*sp = stack;		 
  if (first_time)
  {
    set_code_size = (short)input_code_size;
    code_size     = set_code_size + 1;
    clear_code    = (short)(1 << set_code_size);
    end_code      = clear_code + 1;
    max_code_size = 2 * clear_code;
    max_code      = clear_code + 2;
    gif_get_code(fp, 0, 1);
    fresh = 1;
    for (i = 0; i < clear_code; i ++)
    {
      table[0][i] = 0;
      table[1][i] = (short)i;
    }
    for (; i < 4096; i ++)
      table[0][i] = table[1][0] = 0;
    sp = stack;
    return (0);
  }
  else if (fresh)
  {
    fresh = 0;
    do
      firstcode = oldcode = (short)gif_get_code(fp, code_size, 0);
    while (firstcode == clear_code);
    return (firstcode);
  }
  if (sp > stack)
    return (*--sp);
  while ((code = gif_get_code (fp, code_size, 0)) >= 0)
  {
    if (code == clear_code)
    {
      for (i = 0; i < clear_code; i ++)
      {
	table[0][i] = 0;
	table[1][i] = (short)i;
      }
      for (; i < 4096; i ++)
	table[0][i] = table[1][i] = 0;
      code_size     = set_code_size + 1;
      max_code_size = 2 * clear_code;
      max_code      = clear_code + 2;
      sp = stack;
      firstcode = oldcode = (short)gif_get_code(fp, code_size, 0);
      return (firstcode);
    }
    else if (code == end_code)
    {
      uchar	buf[260];
      if (!gif_eof)
        while (gif_get_block(fp, buf) > 0);
      return (-2);
    }
    incode = code;
    if (code >= max_code)
    {
      *sp++ = firstcode;
      code  = oldcode;
    }
    while (code >= clear_code)
    {
      *sp++ = table[1][code];
      if (code == table[0][code])
	return (255);
      code = table[0][code];
    }
    *sp++ = firstcode = table[1][code];
    code  = max_code;
    if (code < 4096)
    {
      table[0][code] = oldcode;
      table[1][code] = firstcode;
      max_code ++;
      if (max_code >= max_code_size && max_code_size < 4096)
      {
	max_code_size *= 2;
	code_size ++;
      }
    }
    oldcode = (short)incode;
    if (sp > stack)
      return (*--sp);
  }
  return (code);
}