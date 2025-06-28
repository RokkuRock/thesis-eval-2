PixarLogClose(TIFF* tif)
{
	TIFFDirectory *td = &tif->tif_dir;
	td->td_bitspersample = 8;
	td->td_sampleformat = SAMPLEFORMAT_UINT;
}