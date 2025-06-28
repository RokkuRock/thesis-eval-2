int ncrush_decompress(NCRUSH_CONTEXT* ncrush, const BYTE* pSrcData, UINT32 SrcSize,
                      const BYTE** ppDstData, UINT32* pDstSize, UINT32 flags)
{
	UINT32 index;
	UINT32 bits;
	INT32 nbits;
	const BYTE* SrcPtr;
	const BYTE* SrcEnd;
	UINT16 Mask;
	BYTE Literal;
	UINT32 IndexLEC;
	UINT32 BitLength;
	UINT32 MaskedBits;
	UINT32 CopyOffset;
	UINT32 CopyLength;
	UINT32 OldCopyOffset;
	BYTE* CopyOffsetPtr;
	UINT32 LengthOfMatch;
	UINT32 CopyOffsetIndex;
	UINT32 OffsetCacheIndex;
	BYTE* HistoryPtr;
	BYTE* HistoryBuffer;
	BYTE* HistoryBufferEnd;
	UINT32 CopyOffsetBits;
	UINT32 CopyOffsetBase;
	UINT32 LengthOfMatchBits;
	UINT32 LengthOfMatchBase;
	WINPR_ASSERT(ncrush);
	WINPR_ASSERT(pSrcData);
	WINPR_ASSERT(ppDstData);
	WINPR_ASSERT(pDstSize);
	if (ncrush->HistoryEndOffset != 65535)
		return -1001;
	HistoryBuffer = ncrush->HistoryBuffer;
	HistoryBufferEnd = &HistoryBuffer[ncrush->HistoryEndOffset];
	if (flags & PACKET_AT_FRONT)
	{
		if ((ncrush->HistoryPtr - 32768) <= HistoryBuffer)
			return -1002;
		MoveMemory(HistoryBuffer, (ncrush->HistoryPtr - 32768), 32768);
		ncrush->HistoryPtr = &(HistoryBuffer[32768]);
		ZeroMemory(&HistoryBuffer[32768], 32768);
	}
	if (flags & PACKET_FLUSHED)
	{
		ncrush->HistoryPtr = HistoryBuffer;
		ZeroMemory(HistoryBuffer, sizeof(ncrush->HistoryBuffer));
		ZeroMemory(&(ncrush->OffsetCache), sizeof(ncrush->OffsetCache));
	}
	HistoryPtr = ncrush->HistoryPtr;
	if (!(flags & PACKET_COMPRESSED))
	{
		*ppDstData = pSrcData;
		*pDstSize = SrcSize;
		return 1;
	}
	SrcEnd = &pSrcData[SrcSize];
	nbits = 32;
	bits = get_dword(pSrcData);
	SrcPtr = pSrcData + 4;
	while (1)
	{
		while (1)
		{
			Mask = get_word(&HuffTableMask[29]);
			MaskedBits = bits & Mask;
			IndexLEC = HuffTableLEC[MaskedBits] & 0xFFF;
			BitLength = HuffTableLEC[MaskedBits] >> 12;
			bits >>= BitLength;
			nbits -= BitLength;
			if (!NCrushFetchBits(&SrcPtr, &SrcEnd, &nbits, &bits))
				return -1;
			if (IndexLEC >= 256)
				break;
			if (HistoryPtr >= HistoryBufferEnd)
			{
				WLog_ERR(TAG, "ncrush_decompress error: HistoryPtr (%p) >= HistoryBufferEnd (%p)",
				         (void*)HistoryPtr, (void*)HistoryBufferEnd);
				return -1003;
			}
			Literal = (HuffTableLEC[MaskedBits] & 0xFF);
			*HistoryPtr++ = Literal;
		}
		if (IndexLEC == 256)
			break;  
		CopyOffsetIndex = IndexLEC - 257;
		if (CopyOffsetIndex >= 32)
		{
			OffsetCacheIndex = IndexLEC - 289;
			if (OffsetCacheIndex >= 4)
				return -1004;
			CopyOffset = ncrush->OffsetCache[OffsetCacheIndex];
			Mask = get_word(&HuffTableMask[21]);
			MaskedBits = bits & Mask;
			LengthOfMatch = HuffTableLOM[MaskedBits] & 0xFFF;
			BitLength = HuffTableLOM[MaskedBits] >> 12;
			bits >>= BitLength;
			nbits -= BitLength;
			if (!NCrushFetchBits(&SrcPtr, &SrcEnd, &nbits, &bits))
				return -1;
			LengthOfMatchBits = LOMBitsLUT[LengthOfMatch];
			LengthOfMatchBase = LOMBaseLUT[LengthOfMatch];
			if (LengthOfMatchBits)
			{
				Mask = get_word(&HuffTableMask[(2 * LengthOfMatchBits) + 3]);
				MaskedBits = bits & Mask;
				bits >>= LengthOfMatchBits;
				nbits -= LengthOfMatchBits;
				LengthOfMatchBase += MaskedBits;
				if (!NCrushFetchBits(&SrcPtr, &SrcEnd, &nbits, &bits))
					return -1;
			}
			OldCopyOffset = ncrush->OffsetCache[OffsetCacheIndex];
			ncrush->OffsetCache[OffsetCacheIndex] = ncrush->OffsetCache[0];
			ncrush->OffsetCache[0] = OldCopyOffset;
		}
		else
		{
			CopyOffsetBits = CopyOffsetBitsLUT[CopyOffsetIndex];
			CopyOffsetBase = CopyOffsetBaseLUT[CopyOffsetIndex];
			CopyOffset = CopyOffsetBase - 1;
			if (CopyOffsetBits)
			{
				Mask = get_word(&HuffTableMask[(2 * CopyOffsetBits) + 3]);
				MaskedBits = bits & Mask;
				CopyOffset = CopyOffsetBase + MaskedBits - 1;
				bits >>= CopyOffsetBits;
				nbits -= CopyOffsetBits;
				if (!NCrushFetchBits(&SrcPtr, &SrcEnd, &nbits, &bits))
					return -1;
			}
			Mask = get_word(&HuffTableMask[21]);
			MaskedBits = bits & Mask;
			LengthOfMatch = HuffTableLOM[MaskedBits] & 0xFFF;
			BitLength = HuffTableLOM[MaskedBits] >> 12;
			bits >>= BitLength;
			nbits -= BitLength;
			if (!NCrushFetchBits(&SrcPtr, &SrcEnd, &nbits, &bits))
				return -1;
			LengthOfMatchBits = LOMBitsLUT[LengthOfMatch];
			LengthOfMatchBase = LOMBaseLUT[LengthOfMatch];
			if (LengthOfMatchBits)
			{
				Mask = get_word(&HuffTableMask[(2 * LengthOfMatchBits) + 3]);
				MaskedBits = bits & Mask;
				bits >>= LengthOfMatchBits;
				nbits -= LengthOfMatchBits;
				LengthOfMatchBase += MaskedBits;
				if (!NCrushFetchBits(&SrcPtr, &SrcEnd, &nbits, &bits))
					return -1;
			}
			ncrush->OffsetCache[3] = ncrush->OffsetCache[2];
			ncrush->OffsetCache[2] = ncrush->OffsetCache[1];
			ncrush->OffsetCache[1] = ncrush->OffsetCache[0];
			ncrush->OffsetCache[0] = CopyOffset;
		}
		CopyOffsetPtr = &HistoryBuffer[(HistoryPtr - HistoryBuffer - CopyOffset) & 0xFFFF];
		LengthOfMatch = LengthOfMatchBase;
		if (LengthOfMatch < 2)
			return -1005;
		if ((CopyOffsetPtr >= (HistoryBufferEnd - LengthOfMatch)) ||
		    (HistoryPtr >= (HistoryBufferEnd - LengthOfMatch)))
			return -1006;
		CopyOffsetPtr = HistoryPtr - CopyOffset;
		index = 0;
		CopyLength = (LengthOfMatch > CopyOffset) ? CopyOffset : LengthOfMatch;
		if (CopyOffsetPtr >= HistoryBuffer)
		{
			while (CopyLength > 0)
			{
				*HistoryPtr++ = *CopyOffsetPtr++;
				CopyLength--;
			}
			while (LengthOfMatch > CopyOffset)
			{
				index = ((index >= CopyOffset)) ? 0 : index;
				*HistoryPtr++ = *(CopyOffsetPtr + index++);
				LengthOfMatch--;
			}
		}
		else
		{
			CopyOffsetPtr = HistoryBufferEnd - (CopyOffset - (HistoryPtr - HistoryBuffer));
			CopyOffsetPtr++;
			while (CopyLength && (CopyOffsetPtr <= HistoryBufferEnd))
			{
				*HistoryPtr++ = *CopyOffsetPtr++;
				CopyLength--;
			}
			CopyOffsetPtr = HistoryBuffer;
			while (LengthOfMatch > CopyOffset)
			{
				index = ((index >= CopyOffset)) ? 0 : index;
				*HistoryPtr++ = *(CopyOffsetPtr + index++);
				LengthOfMatch--;
			}
		}
		LengthOfMatch = LengthOfMatchBase;
		if (LengthOfMatch == 2)
			continue;
	}
	if (IndexLEC != 256)
		return -1;
	if (ncrush->HistoryBufferFence != 0xABABABAB)
	{
		WLog_ERR(TAG, "NCrushDecompress: history buffer fence was overwritten, potential buffer "
		              "overflow detected!");
		return -1007;
	}
	const intptr_t hsize = HistoryPtr - ncrush->HistoryPtr;
	WINPR_ASSERT(hsize >= 0);
	WINPR_ASSERT(hsize <= UINT32_MAX);
	*pDstSize = (UINT32)hsize;
	*ppDstData = ncrush->HistoryPtr;
	ncrush->HistoryPtr = HistoryPtr;
	return 1;
}