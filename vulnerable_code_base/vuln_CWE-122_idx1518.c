GF_Err gf_isom_get_sample_for_media_time(GF_ISOFile *the_file, u32 trackNumber, u64 desiredTime, u32 *StreamDescriptionIndex, GF_ISOSearchMode SearchMode, GF_ISOSample **sample, u32 *SampleNum, u64 *data_offset)
{
	GF_Err e;
	u32 sampleNumber, prevSampleNumber, syncNum, shadowSync;
	GF_TrackBox *trak;
	GF_ISOSample *shadow;
	GF_SampleTableBox *stbl;
	Bool static_sample = GF_FALSE;
	u8 useShadow, IsSync;
	if (SampleNum) *SampleNum = 0;
	trak = gf_isom_get_track_from_file(the_file, trackNumber);
	if (!trak) return GF_BAD_PARAM;
	stbl = trak->Media->information->sampleTable;
#ifndef	GPAC_DISABLE_ISOM_FRAGMENTS
	if (desiredTime < trak->dts_at_seg_start) {
		desiredTime = 0;
	} else {
		desiredTime -= trak->dts_at_seg_start;
	}
#endif
	e = stbl_findEntryForTime(stbl, desiredTime, 0, &sampleNumber, &prevSampleNumber);
	if (e) return e;
	useShadow = 0;
	if (!stbl->ShadowSync && (SearchMode == GF_ISOM_SEARCH_SYNC_SHADOW))
		SearchMode = GF_ISOM_SEARCH_SYNC_BACKWARD;
	if (! trak->Media->information->sampleTable->SyncSample) {
		if (SearchMode == GF_ISOM_SEARCH_SYNC_FORWARD) SearchMode = GF_ISOM_SEARCH_FORWARD;
		if (SearchMode == GF_ISOM_SEARCH_SYNC_BACKWARD) SearchMode = GF_ISOM_SEARCH_BACKWARD;
	}
	if (!sampleNumber && !prevSampleNumber) {
		if (SearchMode == GF_ISOM_SEARCH_SYNC_BACKWARD || SearchMode == GF_ISOM_SEARCH_BACKWARD) {
			sampleNumber = trak->Media->information->sampleTable->SampleSize->sampleCount;
		}
		if (!sampleNumber) return GF_EOS;
	}
	IsSync = 0;
	switch (SearchMode) {
	case GF_ISOM_SEARCH_SYNC_FORWARD:
		IsSync = 1;
	case GF_ISOM_SEARCH_FORWARD:
		if (!sampleNumber) {
			if (prevSampleNumber != stbl->SampleSize->sampleCount) {
				sampleNumber = prevSampleNumber + 1;
			} else {
				sampleNumber = prevSampleNumber;
			}
		}
		break;
	case GF_ISOM_SEARCH_SYNC_BACKWARD:
		IsSync = 1;
	case GF_ISOM_SEARCH_SYNC_SHADOW:
	case GF_ISOM_SEARCH_BACKWARD:
	default:
		if (!sampleNumber && !prevSampleNumber) {
			sampleNumber = stbl->SampleSize->sampleCount;
		} else if (!sampleNumber) {
			sampleNumber = prevSampleNumber;
		}
		break;
	}
	if (IsSync) {
		e = Media_FindSyncSample(trak->Media->information->sampleTable,
		                         sampleNumber, &syncNum, SearchMode);
		if (e) return e;
		if (syncNum) sampleNumber = syncNum;
		syncNum = 0;
	}
	else if (SearchMode == GF_ISOM_SEARCH_SYNC_SHADOW) {
		e = Media_FindSyncSample(trak->Media->information->sampleTable,
		                         sampleNumber, &syncNum, GF_ISOM_SEARCH_SYNC_BACKWARD);
		if (e) return e;
	}
	if (sample) {
		if (*sample) {
			static_sample = GF_TRUE;
		} else {
			*sample = gf_isom_sample_new();
			if (*sample == NULL) return GF_OUT_OF_MEM;
		}
	}
	if (SearchMode == GF_ISOM_SEARCH_SYNC_SHADOW) {
		stbl_GetSampleShadow(stbl->ShadowSync, &sampleNumber, &shadowSync);
		if ((sampleNumber < syncNum) || (!shadowSync)) {
			sampleNumber = syncNum;
		} else {
			useShadow = 1;
		}
	}
	e = Media_GetSample(trak->Media, sampleNumber, sample, StreamDescriptionIndex, GF_FALSE, data_offset);
	if (e) {
		if (!static_sample)
			gf_isom_sample_del(sample);
		else if (! (*sample)->alloc_size && (*sample)->data && (*sample)->dataLength )
		 	(*sample)->alloc_size =  (*sample)->dataLength;
		return e;
	}
	if (sample && ! (*sample)->IsRAP) {
		Bool is_rap;
		GF_ISOSampleRollType roll_type;
		e = gf_isom_get_sample_rap_roll_info(the_file, trackNumber, sampleNumber, &is_rap, &roll_type, NULL);
		if (e) return e;
		if (is_rap) (*sample)->IsRAP = SAP_TYPE_3;
	}
	if (SampleNum) {
		*SampleNum = sampleNumber;
#ifndef	GPAC_DISABLE_ISOM_FRAGMENTS
		*SampleNum += trak->sample_count_at_seg_start;
#endif
	}
	if (sample && useShadow) {
		shadow = gf_isom_get_sample(the_file, trackNumber, shadowSync, StreamDescriptionIndex);
		if (!shadow) return GF_OK;
		(*sample)->IsRAP = RAP;
		gf_free((*sample)->data);
		(*sample)->dataLength = shadow->dataLength;
		(*sample)->data = shadow->data;
		shadow->dataLength = 0;
		gf_isom_sample_del(&shadow);
	}
	if (static_sample && ! (*sample)->alloc_size )
		 (*sample)->alloc_size =  (*sample)->dataLength;
	return GF_OK;
}