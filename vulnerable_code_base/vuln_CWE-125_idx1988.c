static INLINE BOOL update_read_brush(wStream* s, rdpBrush* brush, BYTE fieldFlags)
{
	if (fieldFlags & ORDER_FIELD_01)
	{
		if (Stream_GetRemainingLength(s) < 1)
			return FALSE;
		Stream_Read_UINT8(s, brush->x);
	}
	if (fieldFlags & ORDER_FIELD_02)
	{
		if (Stream_GetRemainingLength(s) < 1)
			return FALSE;
		Stream_Read_UINT8(s, brush->y);
	}
	if (fieldFlags & ORDER_FIELD_03)
	{
		if (Stream_GetRemainingLength(s) < 1)
			return FALSE;
		Stream_Read_UINT8(s, brush->style);
	}
	if (fieldFlags & ORDER_FIELD_04)
	{
		if (Stream_GetRemainingLength(s) < 1)
			return FALSE;
		Stream_Read_UINT8(s, brush->hatch);
	}
	if (brush->style & CACHED_BRUSH)
	{
		brush->index = brush->hatch;
		brush->bpp = BMF_BPP[brush->style & 0x07];
		if (brush->bpp == 0)
			brush->bpp = 1;
	}
	if (fieldFlags & ORDER_FIELD_05)
	{
		if (Stream_GetRemainingLength(s) < 7)
			return FALSE;
		brush->data = (BYTE*)brush->p8x8;
		Stream_Read_UINT8(s, brush->data[7]);
		Stream_Read_UINT8(s, brush->data[6]);
		Stream_Read_UINT8(s, brush->data[5]);
		Stream_Read_UINT8(s, brush->data[4]);
		Stream_Read_UINT8(s, brush->data[3]);
		Stream_Read_UINT8(s, brush->data[2]);
		Stream_Read_UINT8(s, brush->data[1]);
		brush->data[0] = brush->hatch;
	}
	return TRUE;
}