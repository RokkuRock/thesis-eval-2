BGD_DECLARE(gdImagePtr) gdImageCreateFromGd2Ctx (gdIOCtxPtr in)
{
	int sx, sy;
	int i;
	int ncx, ncy, nc, cs, cx, cy;
	int x, y, ylo, yhi, xlo, xhi;
	int vers, fmt;
	t_chunk_info *chunkIdx = NULL;	 
	unsigned char *chunkBuf = NULL;	 
	int chunkNum = 0;
	int chunkMax = 0;
	uLongf chunkLen;
	int chunkPos = 0;
	int compMax = 0;
	int bytesPerPixel;
	char *compBuf = NULL;		 
	gdImagePtr im;
	im =
	    _gd2CreateFromFile (in, &sx, &sy, &cs, &vers, &fmt, &ncx, &ncy,
	                        &chunkIdx);
	if (im == NULL) {
		return 0;
	}
	bytesPerPixel = im->trueColor ? 4 : 1;
	nc = ncx * ncy;
	if (gd2_compressed (fmt)) {
		compMax = 0;
		for (i = 0; (i < nc); i++) {
			if (chunkIdx[i].size > compMax) {
				compMax = chunkIdx[i].size;
			};
		};
		compMax++;
		chunkMax = cs * bytesPerPixel * cs;
		chunkBuf = gdCalloc (chunkMax, 1);
		if (!chunkBuf) {
			goto fail;
		}
		compBuf = gdCalloc (compMax, 1);
		if (!compBuf) {
			goto fail;
		}
		GD2_DBG (printf ("Largest compressed chunk is %d bytes\n", compMax));
	};
	for (cy = 0; (cy < ncy); cy++) {
		for (cx = 0; (cx < ncx); cx++) {
			ylo = cy * cs;
			yhi = ylo + cs;
			if (yhi > im->sy) {
				yhi = im->sy;
			};
			GD2_DBG (printf
			         ("Processing Chunk %d (%d, %d), y from %d to %d\n",
			          chunkNum, cx, cy, ylo, yhi));
			if (gd2_compressed (fmt)) {
				chunkLen = chunkMax;
				if (!_gd2ReadChunk (chunkIdx[chunkNum].offset,
				                    compBuf,
				                    chunkIdx[chunkNum].size,
				                    (char *) chunkBuf, &chunkLen, in)) {
					GD2_DBG (printf ("Error reading comproessed chunk\n"));
					goto fail;
				};
				chunkPos = 0;
			};
			for (y = ylo; (y < yhi); y++) {
				xlo = cx * cs;
				xhi = xlo + cs;
				if (xhi > im->sx) {
					xhi = im->sx;
				};
				if (!gd2_compressed (fmt)) {
					for (x = xlo; x < xhi; x++) {
						if (im->trueColor) {
							if (!gdGetInt (&im->tpixels[y][x], in)) {
								im->tpixels[y][x] = 0;
							}
						} else {
							int ch;
							if (!gdGetByte (&ch, in)) {
								ch = 0;
							}
							im->pixels[y][x] = ch;
						}
					}
				} else {
					for (x = xlo; x < xhi; x++) {
						if (im->trueColor) {
							int a = chunkBuf[chunkPos++] << 24;
							int r = chunkBuf[chunkPos++] << 16;
							int g = chunkBuf[chunkPos++] << 8;
							int b = chunkBuf[chunkPos++];
							im->tpixels[y][x] = a + r + g + b;
						} else {
							im->pixels[y][x] = chunkBuf[chunkPos++];
						}
					};
				};
			};
			chunkNum++;
		};
	};
	GD2_DBG (printf ("Freeing memory\n"));
	gdFree (chunkBuf);
	gdFree (compBuf);
	gdFree (chunkIdx);
	GD2_DBG (printf ("Done\n"));
	return im;
fail:
	gdImageDestroy (im);
	if (chunkBuf) {
		gdFree (chunkBuf);
	}
	if (compBuf) {
		gdFree (compBuf);
	}
	if (chunkIdx) {
		gdFree (chunkIdx);
	}
	return 0;
}