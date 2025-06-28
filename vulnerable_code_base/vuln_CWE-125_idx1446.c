BOOL update_write_cache_bitmap_v3_order(wStream* s, CACHE_BITMAP_V3_ORDER* cache_bitmap_v3,
                                        UINT16* flags)
{
	BYTE bitsPerPixelId;
	BITMAP_DATA_EX* bitmapData;
	if (!Stream_EnsureRemainingCapacity(
	        s, update_approximate_cache_bitmap_v3_order(cache_bitmap_v3, flags)))
		return FALSE;
	bitmapData = &cache_bitmap_v3->bitmapData;
	bitsPerPixelId = BPP_CBR23[cache_bitmap_v3->bpp];
	*flags = (cache_bitmap_v3->cacheId & 0x00000003) |
	         ((cache_bitmap_v3->flags << 7) & 0x0000FF80) | ((bitsPerPixelId << 3) & 0x00000078);
	Stream_Write_UINT16(s, cache_bitmap_v3->cacheIndex);  
	Stream_Write_UINT32(s, cache_bitmap_v3->key1);        
	Stream_Write_UINT32(s, cache_bitmap_v3->key2);        
	Stream_Write_UINT8(s, bitmapData->bpp);
	Stream_Write_UINT8(s, 0);                    
	Stream_Write_UINT8(s, 0);                    
	Stream_Write_UINT8(s, bitmapData->codecID);  
	Stream_Write_UINT16(s, bitmapData->width);   
	Stream_Write_UINT16(s, bitmapData->height);  
	Stream_Write_UINT32(s, bitmapData->length);  
	Stream_Write(s, bitmapData->data, bitmapData->length);
	return TRUE;
}