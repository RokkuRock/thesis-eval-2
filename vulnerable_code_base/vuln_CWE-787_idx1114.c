bool CxImageTIF::Decode(CxFile * hFile)
{
	TIFF* m_tif = _TIFFOpenEx(hFile, "rb");
	uint32 height=0;
	uint32 width=0;
	uint16 bitspersample=1;
	uint16 samplesperpixel=1;
	uint32 rowsperstrip=(uint32_t)-1;
	uint16 photometric=0;
	uint16 compression=1;
	uint16 orientation=ORIENTATION_TOPLEFT;  
	uint16 res_unit;  
	uint32 x, y;
	float resolution, offset;
	BOOL isRGB;
	uint8_t *bits;		 
	uint8_t *bits2;	 
  cx_try
  {
	if (!m_tif)
		cx_throw("Error encountered while opening TIFF file");
	info.nNumFrames = TIFFNumberOfDirectories(m_tif);
	if (!TIFFSetDirectory(m_tif, (uint16)info.nFrame))
		cx_throw("Error: page not present in TIFF file");			
	TIFFGetField(m_tif, TIFFTAG_IMAGEWIDTH, &width);
	TIFFGetField(m_tif, TIFFTAG_IMAGELENGTH, &height);
	TIFFGetField(m_tif, TIFFTAG_SAMPLESPERPIXEL, &samplesperpixel);
	TIFFGetField(m_tif, TIFFTAG_BITSPERSAMPLE, &bitspersample);
	TIFFGetField(m_tif, TIFFTAG_ROWSPERSTRIP, &rowsperstrip);   
	TIFFGetField(m_tif, TIFFTAG_PHOTOMETRIC, &photometric);
	TIFFGetField(m_tif, TIFFTAG_ORIENTATION, &orientation);
	if (info.nEscape == -1) {
		head.biWidth = width;
		head.biHeight = height;
		info.dwType = CXIMAGE_FORMAT_TIF;
		cx_throw("output dimensions returned");
	}
	TIFFGetFieldDefaulted(m_tif, TIFFTAG_RESOLUTIONUNIT, &res_unit);
	if (TIFFGetField(m_tif, TIFFTAG_XRESOLUTION, &resolution))
	{
		if (res_unit == RESUNIT_CENTIMETER) resolution = (float)(resolution*2.54f + 0.5f);
		SetXDPI((int32_t)resolution);
	}
	if (TIFFGetField(m_tif, TIFFTAG_YRESOLUTION, &resolution))
	{
		if (res_unit == RESUNIT_CENTIMETER) resolution = (float)(resolution*2.54f + 0.5f);
		SetYDPI((int32_t)resolution);
	}
	if (TIFFGetField(m_tif, TIFFTAG_XPOSITION, &offset))	info.xOffset = (int32_t)offset;
	if (TIFFGetField(m_tif, TIFFTAG_YPOSITION, &offset))	info.yOffset = (int32_t)offset;
	head.biClrUsed=0;
	info.nBkgndIndex =-1;
	if (rowsperstrip>height){
		rowsperstrip=height;
		TIFFSetField(m_tif, TIFFTAG_ROWSPERSTRIP, rowsperstrip);
	}
	isRGB =  
		(photometric == PHOTOMETRIC_RGB) ||
		(photometric == PHOTOMETRIC_YCBCR) ||
		(photometric == PHOTOMETRIC_SEPARATED) ||
		(photometric == PHOTOMETRIC_LOGL) ||
		(photometric == PHOTOMETRIC_LOGLUV);
	if (isRGB){
		head.biBitCount=24;
	}else{
		if ((photometric==PHOTOMETRIC_MINISBLACK)||(photometric==PHOTOMETRIC_MINISWHITE)||(photometric==PHOTOMETRIC_PALETTE)){
			if	(bitspersample == 1){
				head.biBitCount=1;		 
				head.biClrUsed =2;
			} else if (bitspersample == 4) {
				head.biBitCount=4;		 
				head.biClrUsed =16;
			} else {
				head.biBitCount=8;		 
				head.biClrUsed =256;
			}
		} else if (bitspersample == 4) {
			head.biBitCount=4;			 
			head.biClrUsed=16;
		} else {
			head.biBitCount=8;			 
			head.biClrUsed=256;
		}
		if ((bitspersample > 8) && (photometric==PHOTOMETRIC_PALETTE))	 
		{	head.biBitCount=24;
			head.biClrUsed =0;
		}
	}
	if (info.nEscape) cx_throw("Cancelled");  
	Create(width,height,head.biBitCount,CXIMAGE_FORMAT_TIF);	 
	if (!pDib) cx_throw("CxImageTIF can't create image");
#if CXIMAGE_SUPPORT_ALPHA
	if (samplesperpixel==4) AlphaCreate();	 
	if (samplesperpixel==2 && bitspersample==8) AlphaCreate();	 
#endif  
	TIFFGetField(m_tif, TIFFTAG_COMPRESSION, &compression);
	SetCodecOption(compression);  
	if (isRGB) {
		uint32* raster;		 
		uint32 *row;
		raster = (uint32*)_TIFFmalloc(width * height * sizeof (uint32));
		if (raster == NULL) cx_throw("No space for raster buffer");
		if(!TIFFReadRGBAImage(m_tif, width, height, raster, 1)) {
				_TIFFfree(raster);
				cx_throw("Corrupted TIFF file!");
		}
		row = &raster[0];
		bits2 = info.pImage;
		for (y = 0; y < height; y++) {
			if (info.nEscape){  
				_TIFFfree(raster);
				cx_throw("Cancelled");
			}
			bits = bits2;
			for (x = 0; x < width; x++) {
				*bits++ = (uint8_t)TIFFGetB(row[x]);
				*bits++ = (uint8_t)TIFFGetG(row[x]);
				*bits++ = (uint8_t)TIFFGetR(row[x]);
#if CXIMAGE_SUPPORT_ALPHA
				if (samplesperpixel==4) AlphaSet(x,y,(uint8_t)TIFFGetA(row[x]));
#endif  
			}
			row += width;
			bits2 += info.dwEffWidth;
		}
		_TIFFfree(raster);
	} else {
		int32_t BIG_palette = (bitspersample > 8) &&	 
						  (photometric==PHOTOMETRIC_PALETTE);		
		if (BIG_palette && (bitspersample > 24))	 
			cx_throw("Too big palette to handle");		 
		RGBQUAD *pal;
		pal=(RGBQUAD*)calloc(BIG_palette ? 1<<bitspersample : 256,sizeof(RGBQUAD)); 
		if (pal==NULL) cx_throw("Unable to allocate TIFF palette");
		int32_t bpp = bitspersample <= 8 ? bitspersample : 8;  
		switch(photometric) {
			case PHOTOMETRIC_MINISBLACK:	 
			case PHOTOMETRIC_MINISWHITE:
				if (bitspersample == 1) {	 
					if (photometric == PHOTOMETRIC_MINISBLACK) {
						pal[1].rgbRed = pal[1].rgbGreen = pal[1].rgbBlue = 255;
					} else {
						pal[0].rgbRed = pal[0].rgbGreen = pal[0].rgbBlue = 255;
					}
				} else {		 
					if (photometric == PHOTOMETRIC_MINISBLACK) {
						for (int32_t i=0; i<(1<<bpp); i++){
							pal[i].rgbRed = pal[i].rgbGreen = pal[i].rgbBlue = (uint8_t)(i*(255/((1<<bpp)-1)));
						}
					} else {
						for (int32_t i=0; i<(1<<bpp); i++){
							pal[i].rgbRed = pal[i].rgbGreen = pal[i].rgbBlue = (uint8_t)(255-i*(255/((1<<bpp)-1)));
						}
					}
				}
				break;
			case PHOTOMETRIC_PALETTE:	 
				uint16 *red;
				uint16 *green;
				uint16 *blue;
				TIFFGetField(m_tif, TIFFTAG_COLORMAP, &red, &green, &blue); 
				BOOL Palette16Bits =   BIG_palette;
				if (!BIG_palette) {
					int32_t n= 1<<bpp;
					while (n-- > 0) {
						if (red[n] >= 256 || green[n] >= 256 || blue[n] >= 256) {
							Palette16Bits=TRUE;
							break;
						}
					}
				}
				for (int32_t i = (1 << ( BIG_palette ? bitspersample : bpp )) - 1; i >= 0; i--) {
					if (Palette16Bits) {
						pal[i].rgbRed =(uint8_t) CVT(red[i]);
						pal[i].rgbGreen = (uint8_t) CVT(green[i]);
						pal[i].rgbBlue = (uint8_t) CVT(blue[i]);           
					} else {
						pal[i].rgbRed = (uint8_t) red[i];
						pal[i].rgbGreen = (uint8_t) green[i];
						pal[i].rgbBlue = (uint8_t) blue[i];        
					}
				}
				break;
		}
		if (!BIG_palette) {  
			SetPalette(pal,  1<<bpp);	 
			free(pal); 
			pal = NULL; 
		}
		uint32 nrow;
		uint32 ys;
		int32_t line = CalculateLine(width, bitspersample * samplesperpixel);
		int32_t bitsize = TIFFStripSize(m_tif);
		if (bitsize>(int32_t)(head.biSizeImage*samplesperpixel))
			bitsize = head.biSizeImage*samplesperpixel;
		if (bitsize<(int32_t)(info.dwEffWidth*rowsperstrip))
			bitsize = info.dwEffWidth*rowsperstrip;
		if ((bitspersample > 8) && (bitspersample != 16))	 
			bitsize *= (bitspersample + 7)/8; 
		int32_t tiled_image = TIFFIsTiled(m_tif);
		uint32 tw=0, tl=0;
		uint8_t* tilebuf=NULL;
		if (tiled_image){
			TIFFGetField(m_tif, TIFFTAG_TILEWIDTH, &tw);
			TIFFGetField(m_tif, TIFFTAG_TILELENGTH, &tl);
			rowsperstrip = tl;
			bitsize = TIFFTileSize(m_tif) * (int32_t)(1+width/tw);
			tilebuf = (uint8_t*)malloc(TIFFTileSize(m_tif));
		}
		bits = (uint8_t*)malloc(bitspersample==16? bitsize*2 : bitsize);  
		uint8_t * bits16 = NULL;										   
		int32_t line16    = 0;											   
		if (!tiled_image && bitspersample==16) {					   
			line16 = line;
			line   = CalculateLine(width, 8 * samplesperpixel);
			bits16 = bits;
			bits   = (uint8_t*)malloc(bitsize);
		}
		if (bits==NULL){
			if (bits16) free(bits16);								   
			if (pal)	free(pal);									   
			if (tilebuf)free(tilebuf);								   
			cx_throw("CxImageTIF can't allocate memory");
		}
#ifdef FIX_16BPP_DARKIMG  
		uint8_t* row_shifts = NULL;
		if (bits16) row_shifts = (uint8_t*)malloc(height); 
#endif
		for (ys = 0; ys < height; ys += rowsperstrip) {
			if (info.nEscape){  
				free(bits);
				cx_throw("Cancelled");
			}
			nrow = (ys + rowsperstrip > height ? height - ys : rowsperstrip);
			if (tiled_image){
				uint32 imagew = TIFFScanlineSize(m_tif);
				uint32 tilew  = TIFFTileRowSize(m_tif);
				int32_t iskew = imagew - tilew;
				uint8* bufp = (uint8*) bits;
				uint32 colb = 0;
				for (uint32 col = 0; col < width; col += tw) {
					if (TIFFReadTile(m_tif, tilebuf, col, ys, 0, 0) < 0){
						free(tilebuf);
						free(bits);
						cx_throw("Corrupted tiled TIFF file!");
					}
					if (colb + tw > imagew) {
						uint32 owidth = imagew - colb;
						uint32 oskew = tilew - owidth;
						TileToStrip(bufp + colb, tilebuf, nrow, owidth, oskew + iskew, oskew );
					} else {
						TileToStrip(bufp + colb, tilebuf, nrow, tilew, iskew, 0);
					}
					colb += tilew;
				}
			} else {
				if (TIFFReadEncodedStrip(m_tif, TIFFComputeStrip(m_tif, ys, 0), 
					(bits16? bits16 : bits), nrow * (bits16 ? line16 : line)) == -1) {  
#ifdef NOT_IGNORE_CORRUPTED
					free(bits);
					if (bits16) free(bits16);   
					cx_throw("Corrupted TIFF file!");
#else
					break;
#endif
				}
			}
			for (y = 0; y < nrow; y++) {
				int32_t offset=(nrow-y-1)*line;
				if ((bitspersample==16) && !BIG_palette) {	 
					int32_t offset16 = (nrow-y-1)*line16;		 
					if (bits16)	{							 
#ifdef FIX_16BPP_DARKIMG
						int32_t the_shift;
						uint8_t hi_byte, hi_max=0;
						uint32_t xi;
						for (xi=0;xi<(uint32)line;xi++) {
							hi_byte = bits16[xi*2+offset16+1];
							if(hi_byte>hi_max)
								hi_max = hi_byte;
						}
						the_shift = (hi_max == 0) ? 8 : 0;
						if (!the_shift)
							while( ! (hi_max & 0x80) ) {
								the_shift++;
								hi_max <<= 1;
							}
						row_shifts[height-ys-nrow+y] = the_shift;
						the_shift = 8 - the_shift;
						for (xi=0;xi<(uint32)line;xi++) 
							bits[xi+offset]= ((bits16[xi*2+offset16+1]<<8) | bits16[xi*2+offset16]) >> the_shift;
#else
						for (uint32_t xi=0;xi<(uint32)line;xi++) 
							bits[xi+offset]=bits16[xi*2+offset16+1];
#endif
					} else {
						for (uint32_t xi=0;xi<width;xi++)
							bits[xi+offset]=bits[xi*2+offset+1];
							}
				}
				if (samplesperpixel==1) { 
					if (BIG_palette)
						if (bits16) {
							int32_t offset16 = (nrow-y-1)*line16;		 
							MoveBitsPal( info.pImage + info.dwEffWidth * (height-ys-nrow+y),
									 bits16 + offset16, width, bitspersample, pal );
						} else
							MoveBitsPal( info.pImage + info.dwEffWidth * (height-ys-nrow+y),
									 bits + offset, width, bitspersample, pal );
					else if ((bitspersample == head.biBitCount) || 
						(bitspersample == 16))	 
						memcpy(info.pImage+info.dwEffWidth*(height-ys-nrow+y),bits+offset,min((unsigned)line, info.dwEffWidth));
					else
						MoveBits( info.pImage + info.dwEffWidth * (height-ys-nrow+y),
								  bits + offset, width, bitspersample );
				} else if (samplesperpixel==2) {  
					int32_t xi=0;
					int32_t ii=0;
					int32_t yi=height-ys-nrow+y;
#if CXIMAGE_SUPPORT_ALPHA
					if (!pAlpha) AlphaCreate();			 
#endif  
					while (ii<line){
						SetPixelIndex(xi,yi,bits[ii+offset]);
#if CXIMAGE_SUPPORT_ALPHA
						AlphaSet(xi,yi,bits[ii+offset+1]);
#endif  
						ii+=2;
						xi++;
						if (xi>=(int32_t)width){
							yi--;
							xi=0;
						}
					}
				} else {  
					if (head.biBitCount!=24){  
						Create(width,height,24,CXIMAGE_FORMAT_TIF);
#if CXIMAGE_SUPPORT_ALPHA
						if (samplesperpixel==4) AlphaCreate();
#endif  
					}
					int32_t xi=0;
					uint32 ii=0;
					int32_t yi=height-ys-nrow+y;
					RGBQUAD c;
					int32_t l,a,b,bitsoffset;
					double p,cx,cy,cz,cr,cg,cb;
					while (ii< width){		 
						bitsoffset = ii*samplesperpixel+offset;
						l=bits[bitsoffset];
						a=bits[bitsoffset+1];
						b=bits[bitsoffset+2];
						if (a>127) a-=256;
						if (b>127) b-=256;
						p = (l/2.55 + 16) / 116.0;
						cx = pow( p + a * 0.002, 3);
						cy = pow( p, 3);
						cz = pow( p - b * 0.005, 3);
						cx*=0.95047;
						cz*=1.0883;
						cr =  3.240479 * cx - 1.537150 * cy - 0.498535 * cz;
						cg = -0.969256 * cx + 1.875992 * cy + 0.041556 * cz;
						cb =  0.055648 * cx - 0.204043 * cy + 1.057311 * cz;
						if ( cr > 0.00304 ) cr = 1.055 * pow(cr,0.41667) - 0.055;
							else            cr = 12.92 * cr;
						if ( cg > 0.00304 ) cg = 1.055 * pow(cg,0.41667) - 0.055;
							else            cg = 12.92 * cg;
						if ( cb > 0.00304 ) cb = 1.055 * pow(cb,0.41667) - 0.055;
							else            cb = 12.92 * cb;
						c.rgbRed  =(uint8_t)max(0,min(255,(int32_t)(cr*255)));
						c.rgbGreen=(uint8_t)max(0,min(255,(int32_t)(cg*255)));
						c.rgbBlue =(uint8_t)max(0,min(255,(int32_t)(cb*255)));
						SetPixelColor(xi,yi,c);
#if CXIMAGE_SUPPORT_ALPHA
						if (samplesperpixel==4) AlphaSet(xi,yi,bits[bitsoffset+3]);
#endif  
						ii++;
						xi++;
						if (xi>=(int32_t)width){
							yi--;
							xi=0;
						}
					}
				}
			}
		}
		free(bits);
		if (bits16) free(bits16);
#ifdef FIX_16BPP_DARKIMG
		if (row_shifts && (samplesperpixel == 1) && (bitspersample==16) && !BIG_palette) {
			int32_t min_row_shift = 8;
			for( y=0; y<height; y++ ) {
				if (min_row_shift > row_shifts[y]) min_row_shift = row_shifts[y];
			}
			for( y=0; y<height; y++ ) {
				if (min_row_shift < row_shifts[y]) {
					int32_t need_shift = row_shifts[y] - min_row_shift;
					uint8_t* data = info.pImage + info.dwEffWidth * y;
					for( x=0; x<width; x++, data++ )
						*data >>= need_shift;
				}
			}
		}
		if (row_shifts)	free( row_shifts );
#endif
		if (tiled_image) free(tilebuf);
		if (pal)		 free(pal);
		switch(orientation){
		case ORIENTATION_TOPRIGHT:  
			Mirror();
			break;
		case ORIENTATION_BOTRIGHT:  
			Flip();
			Mirror();
			break;
		case ORIENTATION_BOTLEFT:  
			Flip();
			break;
		case ORIENTATION_LEFTTOP:  
			RotateRight();
			Mirror();
			break;
		case ORIENTATION_RIGHTTOP:  
			RotateLeft();
			break;
		case ORIENTATION_RIGHTBOT:  
			RotateLeft();
			Mirror();
			break;
		case ORIENTATION_LEFTBOT:  
			RotateRight();
			break;
		}
	}
  } cx_catch {
	  if (strcmp(message,"")) strncpy(info.szLastError,message,255);
	  if (m_tif) TIFFClose(m_tif);
	  if (info.nEscape == -1 && info.dwType == CXIMAGE_FORMAT_TIF) return true;
	  return false;
  }
	TIFFClose(m_tif);
	return true;
}
