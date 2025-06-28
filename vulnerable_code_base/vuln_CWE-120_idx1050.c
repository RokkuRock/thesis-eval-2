GF_Err stbl_AddDTS(GF_SampleTableBox *stbl, u64 DTS, u32 *sampleNumber, u32 LastAUDefDuration, u32 nb_packed_samples)
{
	u32 i, j, sampNum;
	u64 *DTSs, curDTS;
	Bool inserted;
	GF_SttsEntry *ent;
	GF_TimeToSampleBox *stts = stbl->TimeToSample;
	stts->r_FirstSampleInEntry = 0;
	*sampleNumber = 0;
	if (!nb_packed_samples)
		nb_packed_samples=1;
	if (!stts->nb_entries) {
		if (DTS) return GF_BAD_PARAM;
		stts->alloc_size = 1;
		stts->nb_entries = 1;
		stts->entries = gf_malloc(sizeof(GF_SttsEntry));
		if (!stts->entries) return GF_OUT_OF_MEM;
		stts->entries[0].sampleCount = nb_packed_samples;
		stts->entries[0].sampleDelta = (nb_packed_samples>1) ? 0 : LastAUDefDuration;
		(*sampleNumber) = 1;
		stts->w_currentSampleNum = nb_packed_samples;
		return GF_OK;
	}
	if (DTS >= stts->w_LastDTS) {
		u32 nb_extra = 0;
		ent = &stts->entries[stts->nb_entries-1];
		if (!ent->sampleDelta && (ent->sampleCount>1)) {
			ent->sampleDelta = (u32) ( DTS / ent->sampleCount);
			stts->w_LastDTS = DTS - ent->sampleDelta;
		}
		if ((DTS == stts->w_LastDTS + ent->sampleDelta)
			|| ((nb_packed_samples>1) && ((DTS == stts->w_LastDTS) || (DTS == stts->w_LastDTS + 2*ent->sampleDelta) ))
		) {
			(*sampleNumber) = stts->w_currentSampleNum + 1;
			ent->sampleCount += nb_packed_samples;
			stts->w_currentSampleNum += nb_packed_samples;
			stts->w_LastDTS = DTS + ent->sampleDelta * (nb_packed_samples-1);
			return GF_OK;
		}
		if (ent->sampleCount == 1) {
#if 0
			if (stts->w_LastDTS)
				ent->sampleDelta += (u32) (DTS - stts->w_LastDTS);
			else
				ent->sampleDelta = (u32) DTS;
#else
			ent->sampleDelta = (u32) (DTS - stts->w_LastDTS);
#endif
			ent->sampleCount ++;
			if ((stts->nb_entries>=2) && (ent->sampleDelta== stts->entries[stts->nb_entries-2].sampleDelta)) {
				stts->entries[stts->nb_entries-2].sampleCount += ent->sampleCount;
				stts->nb_entries--;
			}
			stts->w_currentSampleNum ++;
			stts->w_LastDTS = DTS;
			(*sampleNumber) = stts->w_currentSampleNum;
			return GF_OK;
		}
		ent->sampleCount --;
		if (nb_packed_samples>1)
			nb_extra = 1;
		if (stts->alloc_size <= stts->nb_entries + nb_extra) {
			ALLOC_INC(stts->alloc_size);
			stts->entries = gf_realloc(stts->entries, sizeof(GF_SttsEntry)*stts->alloc_size);
			if (!stts->entries) return GF_OUT_OF_MEM;
			memset(&stts->entries[stts->nb_entries], 0, sizeof(GF_SttsEntry)*(stts->alloc_size-stts->nb_entries) );
		}
		if (nb_extra)
			nb_extra = stts->entries[stts->nb_entries-1].sampleDelta;
		ent = &stts->entries[stts->nb_entries];
		stts->nb_entries++;
		if (nb_packed_samples==1) {
			ent->sampleCount = 2;
			ent->sampleDelta = (u32) (DTS - stts->w_LastDTS);
			stts->w_LastDTS = DTS;
			(*sampleNumber) = stts->w_currentSampleNum+1;
			stts->w_currentSampleNum += 1;
			return GF_OK;
		}
		ent->sampleCount = 1;
		ent->sampleDelta = (u32) (DTS - stts->w_LastDTS);
		ent = &stts->entries[stts->nb_entries];
		stts->nb_entries++;
		ent->sampleCount = nb_packed_samples;
		ent->sampleDelta = nb_extra;
		stts->w_LastDTS = DTS;
		(*sampleNumber) = stts->w_currentSampleNum + 1;
		stts->w_currentSampleNum += nb_packed_samples;
		return GF_OK;
	}
	DTSs = (u64*)gf_malloc(sizeof(u64) * (stbl->SampleSize->sampleCount+2) );
	if (!DTSs) return GF_OUT_OF_MEM;
	curDTS = 0;
	sampNum = 0;
	ent = NULL;
	inserted = 0;
	for (i=0; i<stts->nb_entries; i++) {
		ent = & stts->entries[i];
		for (j = 0; j<ent->sampleCount; j++) {
			if (!inserted && (curDTS > DTS)) {
				DTSs[sampNum] = DTS;
				sampNum++;
				*sampleNumber = sampNum;
				inserted = 1;
			}
			DTSs[sampNum] = curDTS;
			curDTS += ent->sampleDelta;
			sampNum ++;
		}
	}
	if (!inserted) {
		gf_free(DTSs);
		return GF_BAD_PARAM;
	}
	if (stts->nb_entries+3 >= stts->alloc_size) {
		stts->alloc_size += 3;
		stts->entries = gf_realloc(stts->entries, sizeof(GF_SttsEntry)*stts->alloc_size);
		if (!stts->entries) return GF_OUT_OF_MEM;
		memset(&stts->entries[stts->nb_entries], 0, sizeof(GF_SttsEntry)*(stts->alloc_size - stts->nb_entries) );
	}
	j=0;
	stts->nb_entries = 1;
	stts->entries[0].sampleCount = 1;
	stts->entries[0].sampleDelta = (u32) DTSs[1]  ;
	for (i=1; i<stbl->SampleSize->sampleCount+1; i++) {
		if (i == stbl->SampleSize->sampleCount) {
			stts->entries[j].sampleCount++;
		} else if (stts->entries[j].sampleDelta == (u32) ( DTSs[i+1] - DTSs[i]) ) {
			stts->entries[j].sampleCount ++;
		} else {
			stts->nb_entries ++;
			j++;
			stts->entries[j].sampleCount = 1;
			stts->entries[j].sampleDelta = (u32) (DTSs[i+1] - DTSs[i]);
		}
	}
	gf_free(DTSs);
	stts->w_currentSampleNum = stbl->SampleSize->sampleCount + 1;
	return GF_OK;
}