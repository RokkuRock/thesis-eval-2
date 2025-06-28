static BOOL nsc_rle_decompress_data(NSC_CONTEXT* context)
{
	if (!context)
		return FALSE;
	BYTE* rle = context->Planes;
	WINPR_ASSERT(rle);
	for (size_t i = 0; i < 4; i++)
	{
		const UINT32 originalSize = context->OrgByteCount[i];
		const UINT32 planeSize = context->PlaneByteCount[i];
		if (planeSize == 0)
		{
			if (context->priv->PlaneBuffersLength < originalSize)
				return FALSE;
			FillMemory(context->priv->PlaneBuffers[i], originalSize, 0xFF);
		}
		else if (planeSize < originalSize)
		{
			if (!nsc_rle_decode(rle, context->priv->PlaneBuffers[i],
			                    context->priv->PlaneBuffersLength, originalSize))
				return FALSE;
		}
		else
		{
			if (context->priv->PlaneBuffersLength < originalSize)
				return FALSE;
			CopyMemory(context->priv->PlaneBuffers[i], rle, originalSize);
		}
		rle += planeSize;
	}
	return TRUE;
}