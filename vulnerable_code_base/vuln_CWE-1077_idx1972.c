static u16 swf_get_16(SWFReader *read)
{
	u16 val, res;
	val = swf_read_int(read, 16);
	res = (val&0xFF);
	res <<=8;
	res |= ((val>>8)&0xFF);
	return res;
}