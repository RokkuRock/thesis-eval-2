image_set_mask(image_t *img,	 
               int     x,	 
               int     y,	 
	       uchar   alpha)	 
{
  int		i, j;		 
  uchar		*maskptr;	 
  static uchar	masks[8] =	 
		{
		  0x80, 0x40, 0x20, 0x10,
		  0x08, 0x04, 0x02, 0x01
		};
  static uchar	dither[4][4] =  
		{
		  { 0,  2,  15, 6 },
		  { 4,  12, 9,  11 },
		  { 14, 7,  1,  3 },
		  { 8,  10, 5,  13 }
	        };
  if (img == NULL || img->mask == NULL || x < 0 || x >= img->width ||
      y < 0 || y > img->height)
    return;
  if (img->maskscale == 8)
  {
    if (PSLevel)
      img->mask[y * img->maskwidth + x] = 255 - alpha;
    else
      img->mask[y * img->maskwidth + x] = alpha;
  }
  else
  {
    x *= img->maskscale;
    y *= img->maskscale;
    alpha >>= 4;
    for (i = 0; i < img->maskscale; i ++, y ++, x -= img->maskscale)
      for (j = 0; j < img->maskscale; j ++, x ++)
      {
	maskptr  = img->mask + y * img->maskwidth + x / 8;
	if (alpha <= dither[x & 3][y & 3])
	  *maskptr |= masks[x & 7];
      }
  }
}