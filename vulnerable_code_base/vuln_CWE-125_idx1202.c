static BOOL nsc_rle_decode(BYTE* in, BYTE* out, UINT32 outSize, UINT32 originalSize)
{
	UINT32 left = originalSize;
	while (left > 4)
	{
		const BYTE value = *in++;
		UINT32 len = 0;
		if (left == 5)
		{
			if (outSize < 1)
				return FALSE;
			outSize--;
			*out++ = value;
			left--;
		}
		else if (value == *in)
		{
			in++;
			if (*in < 0xFF)
			{
				len = (UINT32)*in++;
				len += 2;
			}
			else
			{
				in++;
				len = ((UINT32)(*in++));
				len |= ((UINT32)(*in++)) << 8U;
				len |= ((UINT32)(*in++)) << 16U;
				len |= ((UINT32)(*in++)) << 24U;
			}
			if (outSize < len)
				return FALSE;
			outSize -= len;
			FillMemory(out, len, value);
			out += len;
			left -= len;
		}
		else
		{
			if (outSize < 1)
				return FALSE;
			outSize--;
			*out++ = value;
			left--;
		}
	}
	if ((outSize < 4) || (left < 4))
		return FALSE;
	memcpy(out, in, 4);
	return TRUE;
}