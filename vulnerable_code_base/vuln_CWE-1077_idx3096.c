static s16 swf_get_s16(SWFReader *read)
{
	s16 val;
	u8 v1;
	v1 = swf_read_int(read, 8);
	val = swf_read_sint(read, 8);
	val = (val<<8)&0xFF00;
	val |= (v1&0xFF);
	return val;
}