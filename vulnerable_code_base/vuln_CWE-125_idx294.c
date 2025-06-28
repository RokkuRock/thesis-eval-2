void rdp_read_flow_control_pdu(wStream* s, UINT16* type)
{
	UINT8 pduType;
	Stream_Read_UINT8(s, pduType);  
	*type = pduType;
	Stream_Seek_UINT8(s);   
	Stream_Seek_UINT8(s);   
	Stream_Seek_UINT8(s);   
	Stream_Seek_UINT16(s);  
}