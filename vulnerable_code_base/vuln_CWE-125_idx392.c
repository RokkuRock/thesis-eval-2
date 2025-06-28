LogLuvClose(TIFF* tif)
{
	TIFFDirectory *td = &tif->tif_dir;
	td->td_samplesperpixel =
	    (td->td_photometric == PHOTOMETRIC_LOGL) ? 1 : 3;
	td->td_bitspersample = 16;
	td->td_sampleformat = SAMPLEFORMAT_INT;
}