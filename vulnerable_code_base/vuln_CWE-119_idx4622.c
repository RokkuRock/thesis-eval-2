static BOOL rdp_read_font_capability_set(wStream* s, UINT16 length, rdpSettings* settings)
{
	WINPR_UNUSED(settings);
	if (length > 4)
		Stream_Seek_UINT16(s);  
	if (length > 6)
		Stream_Seek_UINT16(s);  
	return TRUE;
}