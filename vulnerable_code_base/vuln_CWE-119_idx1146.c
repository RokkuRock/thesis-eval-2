static void update_read_synchronize(rdpUpdate* update, wStream* s)
{
	WINPR_UNUSED(update);
	Stream_Seek_UINT16(s);  
}