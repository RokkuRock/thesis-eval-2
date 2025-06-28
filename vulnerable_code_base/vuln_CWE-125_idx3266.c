static INLINE BOOL update_write_brush(wStream* s, rdpBrush* brush, BYTE fieldFlags)
{
	if (fieldFlags & ORDER_FIELD_01)
	{
		Stream_Write_UINT8(s, brush->x);
	}
	if (fieldFlags & ORDER_FIELD_02)
	{
		Stream_Write_UINT8(s, brush->y);
	}
	if (fieldFlags & ORDER_FIELD_03)
	{
		Stream_Write_UINT8(s, brush->style);
	}
	if (brush->style & CACHED_BRUSH)
	{
		brush->hatch = brush->index;
		brush->bpp = BMF_BPP[brush->style & 0x07];
		if (brush->bpp == 0)
			brush->bpp = 1;
	}
	if (fieldFlags & ORDER_FIELD_04)
	{
		Stream_Write_UINT8(s, brush->hatch);
	}
	if (fieldFlags & ORDER_FIELD_05)
	{
		brush->data = (BYTE*)brush->p8x8;
		Stream_Write_UINT8(s, brush->data[7]);
		Stream_Write_UINT8(s, brush->data[6]);
		Stream_Write_UINT8(s, brush->data[5]);
		Stream_Write_UINT8(s, brush->data[4]);
		Stream_Write_UINT8(s, brush->data[3]);
		Stream_Write_UINT8(s, brush->data[2]);
		Stream_Write_UINT8(s, brush->data[1]);
		brush->data[0] = brush->hatch;
	}
	return TRUE;
}