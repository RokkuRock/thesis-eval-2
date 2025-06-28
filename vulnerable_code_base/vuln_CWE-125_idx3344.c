BOOL rdp_read_share_control_header(wStream* s, UINT16* length, UINT16* type, UINT16* channel_id)
{
	if (Stream_GetRemainingLength(s) < 2)
		return FALSE;
	Stream_Read_UINT16(s, *length);  
	if (*length == 0x8000)
	{
		rdp_read_flow_control_pdu(s, type);
		*channel_id = 0;
		*length = 8;  
		return TRUE;
	}
	if (((size_t)*length - 2) > Stream_GetRemainingLength(s))
		return FALSE;
	Stream_Read_UINT16(s, *type);  
	*type &= 0x0F;                 
	if (*length > 4)
		Stream_Read_UINT16(s, *channel_id);  
	else
		*channel_id = 0;  
	return TRUE;
}