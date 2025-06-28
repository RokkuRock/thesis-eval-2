BOOL region16_union_rect(REGION16* dst, const REGION16* src, const RECTANGLE_16* rect)
{
	const RECTANGLE_16* srcExtents;
	RECTANGLE_16* dstExtents;
	const RECTANGLE_16* currentBand, *endSrcRect, *nextBand;
	REGION16_DATA* newItems = NULL;
	RECTANGLE_16* dstRect = NULL;
	UINT32 usedRects, srcNbRects;
	UINT16 topInterBand;
	assert(src);
	assert(src->data);
	assert(dst);
	srcExtents = region16_extents(src);
	dstExtents = region16_extents_noconst(dst);
	if (!region16_n_rects(src))
	{
		dst->extents = *rect;
		dst->data = allocateRegion(1);
		if (!dst->data)
			return FALSE;
		dstRect = region16_rects_noconst(dst);
		dstRect->top = rect->top;
		dstRect->left = rect->left;
		dstRect->right = rect->right;
		dstRect->bottom = rect->bottom;
		return TRUE;
	}
	newItems = allocateRegion((1 + region16_n_rects(src)) * 4);
	if (!newItems)
		return FALSE;
	dstRect = (RECTANGLE_16*)(&newItems[1]);
	usedRects = 0;
	if (rect->top < srcExtents->top)
	{
		dstRect->top = rect->top;
		dstRect->left = rect->left;
		dstRect->right = rect->right;
		dstRect->bottom = MIN(srcExtents->top, rect->bottom);
		usedRects++;
		dstRect++;
	}
	currentBand = region16_rects(src, &srcNbRects);
	endSrcRect = currentBand + srcNbRects;
	while (currentBand < endSrcRect)
	{
		if ((currentBand->bottom <= rect->top) || (rect->bottom <= currentBand->top) ||
		    rectangle_contained_in_band(currentBand, endSrcRect, rect))
		{
			region16_copy_band_with_union(dstRect,
			                              currentBand, endSrcRect,
			                              currentBand->top, currentBand->bottom,
			                              NULL, &usedRects,
			                              &nextBand, &dstRect);
			topInterBand = rect->top;
		}
		else
		{
			UINT16 mergeTop = currentBand->top;
			UINT16 mergeBottom = currentBand->bottom;
			if (rect->top > currentBand->top)
			{
				region16_copy_band_with_union(dstRect,
				                              currentBand, endSrcRect,
				                              currentBand->top, rect->top,
				                              NULL, &usedRects,
				                              &nextBand, &dstRect);
				mergeTop = rect->top;
			}
			if (rect->bottom < currentBand->bottom)
				mergeBottom = rect->bottom;
			region16_copy_band_with_union(dstRect,
			                              currentBand, endSrcRect,
			                              mergeTop, mergeBottom,
			                              rect, &usedRects,
			                              &nextBand, &dstRect);
			if (rect->bottom < currentBand->bottom)
			{
				region16_copy_band_with_union(dstRect,
				                              currentBand, endSrcRect,
				                              mergeBottom, currentBand->bottom,
				                              NULL, &usedRects,
				                              &nextBand, &dstRect);
			}
			topInterBand = currentBand->bottom;
		}
		if ((nextBand < endSrcRect) && (nextBand->top != currentBand->bottom) &&
		    (rect->bottom > currentBand->bottom) && (rect->top < nextBand->top))
		{
			dstRect->right = rect->right;
			dstRect->left = rect->left;
			dstRect->top = topInterBand;
			dstRect->bottom = MIN(nextBand->top, rect->bottom);
			dstRect++;
			usedRects++;
		}
		currentBand = nextBand;
	}
	if (srcExtents->bottom < rect->bottom)
	{
		dstRect->top = MAX(srcExtents->bottom, rect->top);
		dstRect->left = rect->left;
		dstRect->right = rect->right;
		dstRect->bottom = rect->bottom;
		usedRects++;
		dstRect++;
	}
	if ((src == dst) && (src->data->size > 0) && (src->data != &empty_region))
		free(src->data);
	dstExtents->top = MIN(rect->top, srcExtents->top);
	dstExtents->left = MIN(rect->left, srcExtents->left);
	dstExtents->bottom = MAX(rect->bottom, srcExtents->bottom);
	dstExtents->right = MAX(rect->right, srcExtents->right);
	newItems->size = sizeof(REGION16_DATA) + (usedRects * sizeof(RECTANGLE_16));
	dst->data = realloc(newItems, newItems->size);
	if (!dst->data)
	{
		free(newItems);
		return FALSE;
	}
	dst->data->nbRects = usedRects;
	return region16_simplify_bands(dst);
}