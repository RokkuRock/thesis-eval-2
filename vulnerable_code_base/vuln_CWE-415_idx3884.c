BGD_DECLARE(void) gdImageJpegCtx(gdImagePtr im, gdIOCtx *outfile, int quality)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	int i, j, jidx;
	volatile JSAMPROW row = 0;
	JSAMPROW rowptr[1];
	jmpbuf_wrapper jmpbufw;
	JDIMENSION nlines;
	char comment[255];
#ifdef JPEG_DEBUG
	gd_error_ex(GD_DEBUG, "gd-jpeg: gd JPEG version %s\n", GD_JPEG_VERSION);
	gd_error_ex(GD_DEBUG, "gd-jpeg: JPEG library version %d, %d-bit sample values\n", JPEG_LIB_VERSION, BITS_IN_JSAMPLE);
	if (!im->trueColor) {
		for(i = 0; i < im->colorsTotal; i++) {
			if(!im->open[i]) {
				gd_error_ex(GD_DEBUG, "gd-jpeg: gd colormap index %d: (%d, %d, %d)\n", i, im->red[i], im->green[i], im->blue[i]);
			}
		}
	}
#endif  
	memset(&cinfo, 0, sizeof(cinfo));
	memset(&jerr, 0, sizeof(jerr));
	cinfo.err = jpeg_std_error(&jerr);
	cinfo.client_data = &jmpbufw;
	if(setjmp(jmpbufw.jmpbuf) != 0) {
		if(row) {
			gdFree(row);
		}
		return;
	}
	cinfo.err->emit_message = jpeg_emit_message;
	cinfo.err->error_exit = fatal_jpeg_error;
	jpeg_create_compress(&cinfo);
	cinfo.image_width = im->sx;
	cinfo.image_height = im->sy;
	cinfo.input_components = 3;  
	cinfo.in_color_space = JCS_RGB;  
	jpeg_set_defaults(&cinfo);
	cinfo.density_unit = 1;
	cinfo.X_density = im->res_x;
	cinfo.Y_density = im->res_y;
	if(quality >= 0) {
		jpeg_set_quality(&cinfo, quality, TRUE);
		if (quality >= 90) {
			cinfo.comp_info[0].h_samp_factor = 1;
			cinfo.comp_info[0].v_samp_factor = 1;
		}
	}
	if(gdImageGetInterlaced(im)) {
#ifdef JPEG_DEBUG
		gd_error_ex(GD_DEBUG, "gd-jpeg: interlace set, outputting progressive JPEG image\n");
#endif
		jpeg_simple_progression(&cinfo);
	}
	jpeg_gdIOCtx_dest(&cinfo, outfile);
	row = (JSAMPROW)gdCalloc(1, cinfo.image_width * cinfo.input_components * sizeof(JSAMPLE));
	if(row == 0) {
		gd_error("gd-jpeg: error: unable to allocate JPEG row structure: gdCalloc returns NULL\n");
		jpeg_destroy_compress(&cinfo);
		return;
	}
	rowptr[0] = row;
	jpeg_start_compress(&cinfo, TRUE);
	sprintf(comment, "CREATOR: gd-jpeg v%s (using IJG JPEG v%d),", GD_JPEG_VERSION, JPEG_LIB_VERSION);
	if(quality >= 0) {
		sprintf (comment + strlen(comment), " quality = %d\n", quality);
	} else {
		strcat(comment + strlen(comment), " default quality\n");
	}
	jpeg_write_marker(&cinfo, JPEG_COM, (unsigned char *) comment, (unsigned int)strlen(comment));
	if(im->trueColor) {
#if BITS_IN_JSAMPLE == 12
		gd_error(
		        "gd-jpeg: error: jpeg library was compiled for 12-bit\n"
		        "precision. This is mostly useless, because JPEGs on the web are\n"
		        "8-bit and such versions of the jpeg library won't read or write\n"
		        "them. GD doesn't support these unusual images. Edit your\n"
		        "jmorecfg.h file to specify the correct precision and completely\n"
		        "'make clean' and 'make install' libjpeg again. Sorry.\n"
		       );
		goto error;
#endif  
		for(i = 0; i < im->sy; i++) {
			for(jidx = 0, j = 0; j < im->sx; j++) {
				int val = im->tpixels[i][j];
				row[jidx++] = gdTrueColorGetRed(val);
				row[jidx++] = gdTrueColorGetGreen(val);
				row[jidx++] = gdTrueColorGetBlue(val);
			}
			nlines = jpeg_write_scanlines(&cinfo, rowptr, 1);
			if(nlines != 1) {
				gd_error("gd_jpeg: warning: jpeg_write_scanlines returns %u -- expected 1\n", nlines);
			}
		}
	} else {
		for(i = 0; i < im->sy; i++) {
			for(jidx = 0, j = 0; j < im->sx; j++) {
				int idx = im->pixels[i][j];
#if BITS_IN_JSAMPLE == 8
				row[jidx++] = im->red[idx];
				row[jidx++] = im->green[idx];
				row[jidx++] = im->blue[idx];
#elif BITS_IN_JSAMPLE == 12
				row[jidx++] = im->red[idx] << 4;
				row[jidx++] = im->green[idx] << 4;
				row[jidx++] = im->blue[idx] << 4;
#else
#error IJG JPEG library BITS_IN_JSAMPLE value must be 8 or 12
#endif
			}
			nlines = jpeg_write_scanlines(&cinfo, rowptr, 1);
			if(nlines != 1) {
				gd_error("gd_jpeg: warning: jpeg_write_scanlines"
				         " returns %u -- expected 1\n", nlines);
			}
		}
	}
	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	gdFree(row);
}