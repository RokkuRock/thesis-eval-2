TIFFSeek(TIFF* tif, uint32 row, uint16 sample )
{
	register TIFFDirectory *td = &tif->tif_dir;
	uint32 strip;
        int    whole_strip;
	tmsize_t read_ahead = 0;
	if (row >= td->td_imagelength) {	 
		TIFFErrorExt(tif->tif_clientdata, tif->tif_name,
		    "%lu: Row out of range, max %lu",
		    (unsigned long) row,
		    (unsigned long) td->td_imagelength);
		return (0);
	}
	if (td->td_planarconfig == PLANARCONFIG_SEPARATE) {
		if (sample >= td->td_samplesperpixel) {
			TIFFErrorExt(tif->tif_clientdata, tif->tif_name,
			    "%lu: Sample out of range, max %lu",
			    (unsigned long) sample, (unsigned long) td->td_samplesperpixel);
			return (0);
		}
		strip = (uint32)sample*td->td_stripsperimage + row/td->td_rowsperstrip;
	} else
		strip = row / td->td_rowsperstrip;
#if defined(CHUNKY_STRIP_READ_SUPPORT)
        if (!_TIFFFillStriles( tif ) || !tif->tif_dir.td_stripbytecount)
            return 0;
        whole_strip = tif->tif_dir.td_stripbytecount[strip] < 10
                || isMapped(tif);
        if( td->td_compression == COMPRESSION_LERC )
        {
            whole_strip = 1;
        }
#else
        whole_strip = 1;
#endif
        if( !whole_strip )
        {
                if( tif->tif_scanlinesize < TIFF_TMSIZE_T_MAX / 16 &&
                    tif->tif_scanlinesize * 16 < TIFF_TMSIZE_T_MAX - 5000 )
                {
                        read_ahead = tif->tif_scanlinesize * 16 + 5000;
                }
                else
                {
                        read_ahead = tif->tif_scanlinesize;
                }
        }
	if (strip != tif->tif_curstrip) {	 
                if( whole_strip )
                {
                        if (!TIFFFillStrip(tif, strip))
                                return (0);
                }
                else
                {
                        if( !TIFFFillStripPartial(tif,strip,read_ahead,1) )
                                return 0;
                }
	}
        else if( !whole_strip )
        {
                if( ((tif->tif_rawdata + tif->tif_rawdataloaded) - tif->tif_rawcp) < read_ahead 
                    && (uint64) tif->tif_rawdataoff+tif->tif_rawdataloaded < td->td_stripbytecount[strip] )
                {
                        if( !TIFFFillStripPartial(tif,strip,read_ahead,0) )
                                return 0;
                }
        }
        if (row < tif->tif_row) {
                if( tif->tif_rawdataoff != 0 )
                {
                        if( !TIFFFillStripPartial(tif,strip,read_ahead,1) )
                                return 0;
                }
                else
                {
                        if (!TIFFStartStrip(tif, strip))
                                return (0);
                }
	}
	if (row != tif->tif_row) {
		if (!(*tif->tif_seek)(tif, row - tif->tif_row))
			return (0);
		tif->tif_row = row;
	}
	return (1);
}