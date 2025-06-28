int ntlm_read_message_fields(wStream* s, NTLM_MESSAGE_FIELDS* fields)
{
	if (Stream_GetRemainingLength(s) < 8)
		return -1;
	Stream_Read_UINT16(s, fields->Len);  
	Stream_Read_UINT16(s, fields->MaxLen);  
	Stream_Read_UINT32(s, fields->BufferOffset);  
	return 1;
}