image_load_bmp(image_t *img,	 
               FILE    *fp,	 
	       int     gray,	 
               int     load_data) 
{
  int		info_size,	 
		depth,		 
		compression,	 
		colors_used,	 
		x, y,		 
		color,		 
		count,		 
		temp,		 
		align;		 
  uchar		bit,		 
		byte;		 
  uchar		*ptr;		 
  uchar		colormap[256][4]; 
  getc(fp);			 
  getc(fp);
  read_dword(fp);		 
  read_word(fp);		 
  read_word(fp);
  read_dword(fp);
  info_size        = (int)read_dword(fp);
  img->width       = read_long(fp);
  img->height      = read_long(fp);
  read_word(fp);
  depth            = read_word(fp);
  compression      = (int)read_dword(fp);
  read_dword(fp);
  read_long(fp);
  read_long(fp);
  colors_used      = (int)read_dword(fp);
  read_dword(fp);
  if (info_size > 40)
    for (info_size -= 40; info_size > 0; info_size --)
      getc(fp);
  if (colors_used == 0 && depth <= 8)
    colors_used = 1 << depth;
  fread(colormap, (size_t)colors_used, 4, fp);
  img->depth  = gray ? 1 : 3;
  if (depth <= 8 && Encryption)
    img->use ++;
  if (!load_data)
    return (0);
  img->pixels = (uchar *)malloc((size_t)(img->width * img->height * img->depth));
  if (img->pixels == NULL)
    return (-1);
  if (gray && depth <= 8)
  {
    for (color = colors_used - 1; color >= 0; color --)
      colormap[color][0] = (colormap[color][2] * 31 +
                            colormap[color][1] * 61 +
                            colormap[color][0] * 8) / 100;
  }
  color = 0;
  count = 0;
  align = 0;
  byte  = 0;
  temp  = 0;
  for (y = img->height - 1; y >= 0; y --)
  {
    ptr = img->pixels + y * img->width * img->depth;
    switch (depth)
    {
      case 1 :  
          for (x = img->width, bit = 128; x > 0; x --)
	  {
	    if (bit == 128)
	      byte = (uchar)getc(fp);
	    if (byte & bit)
	    {
	      if (!gray)
	      {
		*ptr++ = colormap[1][2];
		*ptr++ = colormap[1][1];
              }
	      *ptr++ = colormap[1][0];
	    }
	    else
	    {
	      if (!gray)
	      {
		*ptr++ = colormap[0][2];
		*ptr++ = colormap[0][1];
	      }
	      *ptr++ = colormap[0][0];
	    }
	    if (bit > 1)
	      bit >>= 1;
	    else
	      bit = 128;
	  }
	  for (temp = (img->width + 7) / 8; temp & 3; temp ++)
	    getc(fp);
          break;
      case 4 :  
          for (x = img->width, bit = 0xf0; x > 0; x --)
	  {
            if (compression != BI_RLE4 && count == 0)
	    {
	      count = 2;
	      color = -1;
            }
	    if (count == 0)
	    {
	      while (align > 0)
	      {
	        align --;
		getc(fp);
              }
	      if ((count = getc(fp)) == 0)
	      {
		if ((count = getc(fp)) == 0)
		{
                  x ++;
		  continue;
		}
		else if (count == 1)
		{
		  break;
		}
		else if (count == 2)
		{
		  count = getc(fp) * getc(fp) * img->width;
		  color = 0;
		}
		else
		{
		  color = -1;
		  align = ((4 - (count & 3)) / 2) & 1;
		}
	      }
	      else
	        color = getc(fp);
            }
	    count --;
            if (bit == 0xf0)
	    {
              if (color < 0)
		temp = getc(fp);
	      else
		temp = color;
              if (!gray)
	      {
		*ptr++ = colormap[temp >> 4][2];
		*ptr++ = colormap[temp >> 4][1];
              }
	      *ptr++ = colormap[temp >> 4][0];
	      bit    = 0x0f;
            }
	    else
	    {
	      if (!gray)
	      {
	        *ptr++ = colormap[temp & 15][2];
	        *ptr++ = colormap[temp & 15][1];
	      }
	      *ptr++ = colormap[temp & 15][0];
	      bit    = 0xf0;
	    }
	  }
          break;
      case 8 :  
          for (x = img->width; x > 0; x --)
	  {
            if (compression != BI_RLE8)
	    {
	      count = 1;
	      color = -1;
            }
	    if (count == 0)
	    {
	      while (align > 0)
	      {
	        align --;
		getc(fp);
              }
	      if ((count = getc(fp)) == 0)
	      {
		if ((count = getc(fp)) == 0)
		{
                  x ++;
		  continue;
		}
		else if (count == 1)
		{
		  break;
		}
		else if (count == 2)
		{
		  count = getc(fp) * getc(fp) * img->width;
		  color = 0;
		}
		else
		{
		  color = -1;
		  align = (2 - (count & 1)) & 1;
		}
	      }
	      else
	        color = getc(fp);
            }
            if (color < 0)
	      temp = getc(fp);
	    else
	      temp = color;
            count --;
            if (!gray)
	    {
	      *ptr++ = colormap[temp][2];
	      *ptr++ = colormap[temp][1];
	    }
	    *ptr++ = colormap[temp][0];
	  }
          break;
      case 24 :  
          if (gray)
	  {
            for (x = img->width; x > 0; x --)
	    {
	      temp = getc(fp) * 8;
	      temp += getc(fp) * 61;
	      temp += getc(fp) * 31;
	      *ptr++ = (uchar)(temp / 100);
	    }
	  }
	  else
	  {
            for (x = img->width; x > 0; x --, ptr += 3)
	    {
	      ptr[2] = (uchar)getc(fp);
	      ptr[1] = (uchar)getc(fp);
	      ptr[0] = (uchar)getc(fp);
	    }
          }
	  for (temp = img->width * 3; temp & 3; temp ++)
	    getc(fp);
          break;
    }
  }
  return (0);
}