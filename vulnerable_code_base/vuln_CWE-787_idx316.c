image_load_gif(image_t *img,	 
               FILE    *fp,	 
               int     gray,	 
               int     load_data) 
{
  uchar		buf[1024];	 
  gif_cmap_t	cmap;		 
  int		ncolors,	 
		transparent;	 
  fread(buf, 13, 1, fp);
  img->width  = (buf[7] << 8) | buf[6];
  img->height = (buf[9] << 8) | buf[8];
  ncolors     = 2 << (buf[10] & 0x07);
  if (img->width <= 0 || img->width > 32767 || img->height <= 0 || img->height > 32767)
    return (-1);
  if (Encryption)
    img->use ++;
  if (buf[10] & GIF_COLORMAP)
    if (gif_read_cmap(fp, ncolors, cmap, &gray))
      return (-1);
  transparent = -1;
  while (1)
  {
    switch (getc(fp))
    {
      case ';' :	 
          return (-1);		 
      case '!' :	 
          buf[0] = (uchar)getc(fp);
          if (buf[0] == 0xf9)	 
          {
            gif_get_block(fp, buf);
            if (buf[0] & 1)	 
              transparent = buf[3];
          }
          while (gif_get_block(fp, buf) != 0);
          break;
      case ',' :	 
          fread(buf, 9, 1, fp);
          if (buf[8] & GIF_COLORMAP)
          {
            ncolors = 2 << (buf[8] & 0x07);
	    if (gif_read_cmap(fp, ncolors, cmap, &gray))
	      return (-1);
	  }
          if (transparent >= 0)
          {
            if (BodyColor[0])
	    {
	      float rgb[3];  
	      get_color((uchar *)BodyColor, rgb);
	      cmap[transparent][0] = (uchar)(rgb[0] * 255.0f + 0.5f);
	      cmap[transparent][1] = (uchar)(rgb[1] * 255.0f + 0.5f);
	      cmap[transparent][2] = (uchar)(rgb[2] * 255.0f + 0.5f);
	    }
	    else
	    {
	      cmap[transparent][0] = 255;
              cmap[transparent][1] = 255;
              cmap[transparent][2] = 255;
	    }
            image_need_mask(img);
	  }
          img->width  = (buf[5] << 8) | buf[4];
          img->height = (buf[7] << 8) | buf[6];
          img->depth  = gray ? 1 : 3;
	  if (img->width <= 0 || img->width > 32767 || img->height <= 0 || img->height > 32767)
	    return (-1);
	  if (!load_data)
	    return (0);
          img->pixels = (uchar *)malloc((size_t)(img->width * img->height * img->depth));
          if (img->pixels == NULL)
            return (-1);
	  return (gif_read_image(fp, img, cmap, buf[8] & GIF_INTERLACE, transparent));
    }
  }
}