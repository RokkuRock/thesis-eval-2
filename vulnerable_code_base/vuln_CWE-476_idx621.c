image_load_jpeg(image_t *img,	 
                FILE    *fp,	 
                int     gray,	 
                int     load_data) 
{
  struct jpeg_decompress_struct	cinfo;		 
  struct jpeg_error_mgr		jerr;		 
  JSAMPROW			row;		 
  jpeg_std_error(&jerr);
  jerr.error_exit = jpeg_error_handler;
  cinfo.err = &jerr;
  jpeg_create_decompress(&cinfo);
  jpeg_stdio_src(&cinfo, fp);
  jpeg_read_header(&cinfo, (boolean)1);
  cinfo.quantize_colors = FALSE;
  if (gray || cinfo.num_components == 1)
  {
    cinfo.out_color_space      = JCS_GRAYSCALE;
    cinfo.out_color_components = 1;
    cinfo.output_components    = 1;
  }
  else if (cinfo.num_components != 3)
  {
    jpeg_destroy_decompress(&cinfo);
    progress_error(HD_ERROR_BAD_FORMAT,
                   "CMYK JPEG files are not supported! (%s)",
		   file_rlookup(img->filename));
    return (-1);
  }
  else
  {
    cinfo.out_color_space      = JCS_RGB;
    cinfo.out_color_components = 3;
    cinfo.output_components    = 3;
  }
  jpeg_calc_output_dimensions(&cinfo);
  img->width  = (int)cinfo.output_width;
  img->height = (int)cinfo.output_height;
  img->depth  = (int)cinfo.output_components;
  if (!load_data)
  {
    jpeg_destroy_decompress(&cinfo);
    return (0);
  }
  img->pixels = (uchar *)malloc((size_t)(img->width * img->height * img->depth));
  if (img->pixels == NULL)
  {
    jpeg_destroy_decompress(&cinfo);
    return (-1);
  }
  jpeg_start_decompress(&cinfo);
  while (cinfo.output_scanline < cinfo.output_height)
  {
    row = (JSAMPROW)(img->pixels + (size_t)cinfo.output_scanline * (size_t)cinfo.output_width * (size_t)cinfo.output_components);
    jpeg_read_scanlines(&cinfo, &row, (JDIMENSION)1);
  }
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  return (0);
}