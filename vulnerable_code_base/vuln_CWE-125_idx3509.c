start_input_ppm(j_compress_ptr cinfo, cjpeg_source_ptr sinfo)
{
  ppm_source_ptr source = (ppm_source_ptr)sinfo;
  int c;
  unsigned int w, h, maxval;
  boolean need_iobuffer, use_raw_buffer, need_rescale;
  if (getc(source->pub.input_file) != 'P')
    ERREXIT(cinfo, JERR_PPM_NOT);
  c = getc(source->pub.input_file);  
  switch (c) {
  case '2':                      
  case '3':                      
  case '5':                      
  case '6':                      
    break;
  default:
    ERREXIT(cinfo, JERR_PPM_NOT);
    break;
  }
  w = read_pbm_integer(cinfo, source->pub.input_file, 65535);
  h = read_pbm_integer(cinfo, source->pub.input_file, 65535);
  maxval = read_pbm_integer(cinfo, source->pub.input_file, 65535);
  if (w <= 0 || h <= 0 || maxval <= 0)  
    ERREXIT(cinfo, JERR_PPM_NOT);
  cinfo->data_precision = BITS_IN_JSAMPLE;  
  cinfo->image_width = (JDIMENSION)w;
  cinfo->image_height = (JDIMENSION)h;
  source->maxval = maxval;
  need_iobuffer = TRUE;          
  use_raw_buffer = FALSE;        
  need_rescale = TRUE;           
  switch (c) {
  case '2':                      
    if (cinfo->in_color_space == JCS_UNKNOWN)
      cinfo->in_color_space = JCS_GRAYSCALE;
    TRACEMS2(cinfo, 1, JTRC_PGM_TEXT, w, h);
    if (cinfo->in_color_space == JCS_GRAYSCALE)
      source->pub.get_pixel_rows = get_text_gray_row;
    else if (IsExtRGB(cinfo->in_color_space))
      source->pub.get_pixel_rows = get_text_gray_rgb_row;
    else if (cinfo->in_color_space == JCS_CMYK)
      source->pub.get_pixel_rows = get_text_gray_cmyk_row;
    else
      ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    need_iobuffer = FALSE;
    break;
  case '3':                      
    if (cinfo->in_color_space == JCS_UNKNOWN)
      cinfo->in_color_space = JCS_EXT_RGB;
    TRACEMS2(cinfo, 1, JTRC_PPM_TEXT, w, h);
    if (IsExtRGB(cinfo->in_color_space))
      source->pub.get_pixel_rows = get_text_rgb_row;
    else if (cinfo->in_color_space == JCS_CMYK)
      source->pub.get_pixel_rows = get_text_rgb_cmyk_row;
    else
      ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    need_iobuffer = FALSE;
    break;
  case '5':                      
    if (cinfo->in_color_space == JCS_UNKNOWN)
      cinfo->in_color_space = JCS_GRAYSCALE;
    TRACEMS2(cinfo, 1, JTRC_PGM, w, h);
    if (maxval > 255) {
      source->pub.get_pixel_rows = get_word_gray_row;
    } else if (maxval == MAXJSAMPLE && sizeof(JSAMPLE) == sizeof(U_CHAR) &&
               cinfo->in_color_space == JCS_GRAYSCALE) {
      source->pub.get_pixel_rows = get_raw_row;
      use_raw_buffer = TRUE;
      need_rescale = FALSE;
    } else {
      if (cinfo->in_color_space == JCS_GRAYSCALE)
        source->pub.get_pixel_rows = get_scaled_gray_row;
      else if (IsExtRGB(cinfo->in_color_space))
        source->pub.get_pixel_rows = get_gray_rgb_row;
      else if (cinfo->in_color_space == JCS_CMYK)
        source->pub.get_pixel_rows = get_gray_cmyk_row;
      else
        ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    }
    break;
  case '6':                      
    if (cinfo->in_color_space == JCS_UNKNOWN)
      cinfo->in_color_space = JCS_EXT_RGB;
    TRACEMS2(cinfo, 1, JTRC_PPM, w, h);
    if (maxval > 255) {
      source->pub.get_pixel_rows = get_word_rgb_row;
    } else if (maxval == MAXJSAMPLE && sizeof(JSAMPLE) == sizeof(U_CHAR) &&
               (cinfo->in_color_space == JCS_EXT_RGB
#if RGB_RED == 0 && RGB_GREEN == 1 && RGB_BLUE == 2 && RGB_PIXELSIZE == 3
                || cinfo->in_color_space == JCS_RGB
#endif
               )) {
      source->pub.get_pixel_rows = get_raw_row;
      use_raw_buffer = TRUE;
      need_rescale = FALSE;
    } else {
      if (IsExtRGB(cinfo->in_color_space))
        source->pub.get_pixel_rows = get_rgb_row;
      else if (cinfo->in_color_space == JCS_CMYK)
        source->pub.get_pixel_rows = get_rgb_cmyk_row;
      else
        ERREXIT(cinfo, JERR_BAD_IN_COLORSPACE);
    }
    break;
  }
  if (IsExtRGB(cinfo->in_color_space))
    cinfo->input_components = rgb_pixelsize[cinfo->in_color_space];
  else if (cinfo->in_color_space == JCS_GRAYSCALE)
    cinfo->input_components = 1;
  else if (cinfo->in_color_space == JCS_CMYK)
    cinfo->input_components = 4;
  if (need_iobuffer) {
    if (c == '6')
      source->buffer_width = (size_t)w * 3 *
        ((maxval <= 255) ? sizeof(U_CHAR) : (2 * sizeof(U_CHAR)));
    else
      source->buffer_width = (size_t)w *
        ((maxval <= 255) ? sizeof(U_CHAR) : (2 * sizeof(U_CHAR)));
    source->iobuffer = (U_CHAR *)
      (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                                  source->buffer_width);
  }
  if (use_raw_buffer) {
    source->pixrow = (JSAMPROW)source->iobuffer;
    source->pub.buffer = &source->pixrow;
    source->pub.buffer_height = 1;
  } else {
    source->pub.buffer = (*cinfo->mem->alloc_sarray)
      ((j_common_ptr)cinfo, JPOOL_IMAGE,
       (JDIMENSION)w * cinfo->input_components, (JDIMENSION)1);
    source->pub.buffer_height = 1;
  }
  if (need_rescale) {
    long val, half_maxval;
    source->rescale = (JSAMPLE *)
      (*cinfo->mem->alloc_small) ((j_common_ptr)cinfo, JPOOL_IMAGE,
                                  (size_t)(((long)maxval + 1L) *
                                           sizeof(JSAMPLE)));
    half_maxval = maxval / 2;
    for (val = 0; val <= (long)maxval; val++) {
      source->rescale[val] = (JSAMPLE)((val * MAXJSAMPLE + half_maxval) /
                                        maxval);
    }
  }
}