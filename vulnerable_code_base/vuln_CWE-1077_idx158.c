static u32 swf_get_32(SWFReader *read)
{
	u32 val, res;
	val = swf_read_int(read, 32);
	res = (val&0xFF);
	res <<=8;
	res |= ((val>>8)&0xFF);
	res<<=8;
	res |= ((val>>16)&0xFF);
	res<<=8;
	res|= ((val>>24)&0xFF);
	return res;
}