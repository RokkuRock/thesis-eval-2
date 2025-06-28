GF_Err gf_isom_fragment_add_sample_ex(GF_ISOFile *movie, GF_ISOTrackID TrackID, const GF_ISOSample *sample, u32 DescIndex,
                                   u32 Duration, u8 PaddingBits, u16 DegradationPriority, Bool redundant_coding, void **ref, u32 ref_offset)
{
	u32 count, buffer_size;
	u8 *buffer;
	u64 pos;
	GF_ISOSample *od_sample = NULL;
	GF_TrunEntry ent, *prev_ent;
	GF_TrackFragmentBox *traf, *traf_2;
	GF_TrackFragmentRunBox *trun;
	if (!movie->moof || !(movie->FragmentsFlags & GF_ISOM_FRAG_WRITE_READY) || !sample)
		return GF_BAD_PARAM;
	traf = gf_isom_get_traf(movie, TrackID);
	if (!traf)
		return GF_BAD_PARAM;
	if (!traf->tfhd->sample_desc_index)
		traf->tfhd->sample_desc_index = DescIndex ? DescIndex : traf->trex->def_sample_desc_index;
	pos = gf_bs_get_position(movie->editFileMap->bs);
	if ( DescIndex && (traf->tfhd->sample_desc_index != DescIndex)) {
		if (traf->DataCache && !traf->use_sample_interleave) {
			count = gf_list_count(traf->TrackRuns);
			if (count) {
				trun = (GF_TrackFragmentRunBox *)gf_list_get(traf->TrackRuns, count-1);
				trun->data_offset = (u32) (pos - movie->moof->fragment_offset - 8);
				gf_bs_get_content(trun->cache, &buffer, &buffer_size);
				gf_bs_write_data(movie->editFileMap->bs, buffer, buffer_size);
				gf_bs_del(trun->cache);
				trun->cache = NULL;
				gf_free(buffer);
			}
		}
		traf_2 = (GF_TrackFragmentBox *) gf_isom_box_new_parent(&movie->moof->child_boxes, GF_ISOM_BOX_TYPE_TRAF);
		if (!traf_2) return GF_OUT_OF_MEM;
		traf_2->trex = traf->trex;
		traf_2->tfhd = (GF_TrackFragmentHeaderBox *) gf_isom_box_new_parent(&traf_2->child_boxes, GF_ISOM_BOX_TYPE_TFHD);
		if (!traf_2->tfhd) return GF_OUT_OF_MEM;
		traf_2->tfhd->trackID = traf->tfhd->trackID;
		traf_2->tfhd->base_data_offset = movie->moof->fragment_offset + 8;
		gf_list_add(movie->moof->TrackList, traf_2);
		traf_2->IFrameSwitching = traf->IFrameSwitching;
		traf_2->use_sample_interleave = traf->use_sample_interleave;
		traf_2->interleave_id = traf->interleave_id;
		traf_2->truns_first = traf->truns_first;
		traf_2->truns_v1 = traf->truns_v1;
		traf_2->large_tfdt = traf->large_tfdt;
		traf_2->DataCache  = traf->DataCache;
		traf_2->tfhd->sample_desc_index  = DescIndex;
		traf = traf_2;
	}
	pos = movie->moof->trun_ref_size ? (8+movie->moof->trun_ref_size) : gf_bs_get_position(movie->editFileMap->bs);
	count = (traf->use_sample_interleave && traf->force_new_trun) ? 0 : gf_list_count(traf->TrackRuns);
	if (count) {
		trun = (GF_TrackFragmentRunBox *)gf_list_get(traf->TrackRuns, count-1);
		if (!traf->DataCache && (movie->moof->fragment_offset + 8 + trun->data_offset + trun->run_size != pos) )
			count = 0;
		if (traf->IFrameSwitching && sample->IsRAP)
			count = 0;
		if (traf->DataCache && (traf->DataCache==trun->sample_count) && !traf->use_sample_interleave)
			count = 0;
		if (traf->force_new_trun)
			count = 0;
		if (!count && traf->DataCache && !traf->use_sample_interleave) {
			trun->data_offset = (u32) (pos - movie->moof->fragment_offset - 8);
			gf_bs_get_content(trun->cache, &buffer, &buffer_size);
			gf_bs_write_data(movie->editFileMap->bs, buffer, buffer_size);
			gf_bs_del(trun->cache);
			trun->cache = NULL;
			gf_free(buffer);
		}
	}
	traf->force_new_trun = 0;
	if (!count) {
		trun = (GF_TrackFragmentRunBox *) gf_isom_box_new_parent(&traf->child_boxes, GF_ISOM_BOX_TYPE_TRUN);
		if (!trun) return GF_OUT_OF_MEM;
		trun->data_offset = (u32) (pos - movie->moof->fragment_offset - 8);
		gf_list_add(traf->TrackRuns, trun);
#ifdef GF_ENABLE_CTRN
		trun->use_ctrn = traf->use_ctrn;
		trun->use_inherit = traf->use_inherit;
		trun->ctso_multiplier = traf->trex->def_sample_duration;
#endif
		trun->interleave_id = traf->interleave_id;
		if (traf->truns_v1)
			trun->version = 1;
		if (traf->DataCache)
			trun->cache = gf_bs_new(NULL, 0, GF_BITSTREAM_WRITE);
	}
	memset(&ent, 0, sizeof(GF_TrunEntry));
	ent.CTS_Offset = sample->CTS_Offset;
	ent.Duration = Duration;
	ent.dts = sample->DTS;
	ent.nb_pack = sample->nb_pack;
	ent.flags = GF_ISOM_FORMAT_FRAG_FLAGS(PaddingBits, sample->IsRAP, DegradationPriority);
	if (sample->IsRAP) {
		ent.flags |= GF_ISOM_GET_FRAG_DEPEND_FLAGS(0, 2, 0, (redundant_coding ? 1 : 0) );
		ent.SAP_type = sample->IsRAP;
	}
	if (trun->nb_samples) {
		prev_ent = &trun->samples[trun->nb_samples-1];
	} else {
		prev_ent = NULL;
	}
	if (prev_ent && (prev_ent->dts || !prev_ent->Duration) && sample->DTS) {
		u32 nsamp = prev_ent->nb_pack ? prev_ent->nb_pack : 1;
		if (nsamp*prev_ent->Duration != sample->DTS - prev_ent->dts)
			prev_ent->Duration = (u32) (sample->DTS - prev_ent->dts) / nsamp;
	}
	if (trun->nb_samples >= trun->sample_alloc) {
		trun->sample_alloc += 50;
		if (trun->nb_samples >= trun->sample_alloc) trun->sample_alloc = trun->nb_samples+1;
		trun->samples = gf_realloc(trun->samples, sizeof(GF_TrunEntry)*trun->sample_alloc);
		if (!trun->samples) return GF_OUT_OF_MEM;
	}
	if (traf->trex->track->Media->handler->handlerType == GF_ISOM_MEDIA_OD) {
		Media_ParseODFrame(traf->trex->track->Media, sample, &od_sample);
		sample = od_sample;
	}
	ent.size = sample->dataLength;
	trun->samples[trun->nb_samples] = ent;
	trun->nb_samples ++;
	trun->run_size += ent.size;
	if (sample->CTS_Offset<0) {
		trun->version = 1;
	}
	trun->sample_count += sample->nb_pack ? sample->nb_pack : 1;
	if (sample->dataLength) {
		u32 res = 0;
		if (!traf->DataCache) {
			if (movie->moof_first && movie->on_block_out && (ref || trun->sample_refs)) {
				GF_TrafSampleRef *sref;
				if (!trun->sample_refs) trun->sample_refs = gf_list_new();
				GF_SAFEALLOC(sref, GF_TrafSampleRef);
				if (!sref) return GF_OUT_OF_MEM;
				if (ref && *ref && !od_sample) {
					sref->data = sample->data;
					sref->len = sample->dataLength;
					sref->ref = *ref;
					sref->ref_offset = ref_offset;
					*ref = NULL;
				} else {
					sref->data = gf_malloc(sample->dataLength);
					if (!sref->data) {
						gf_free(sref);
						return GF_OUT_OF_MEM;
					}
					memcpy(sref->data, sample->data, sample->dataLength);
					sref->len = sample->dataLength;
				}
				res = sref->len;
				traf->trun_ref_size += res;
				movie->moof->trun_ref_size += res;
				gf_list_add(trun->sample_refs, sref);
			} else {
				res = gf_bs_write_data(movie->editFileMap->bs, sample->data, sample->dataLength);
			}
		} else if (trun->cache) {
			res = gf_bs_write_data(trun->cache, sample->data, sample->dataLength);
		} else {
			return GF_BAD_PARAM;
		}
		if (res!=sample->dataLength) {
			GF_LOG(GF_LOG_WARNING, GF_LOG_CONTAINER, ("[iso fragment] Could not add a sample with a size of %u bytes\n", sample->dataLength));
			return GF_OUT_OF_MEM;
		}
	}
	if (od_sample) gf_isom_sample_del(&od_sample);
	if (traf->trex->tfra) {
		GF_RandomAccessEntry *raf;
		raf = &traf->trex->tfra->entries[traf->trex->tfra->nb_entries-1];
		if (!raf->trun_number && sample->IsRAP) {
			raf->time = sample->DTS + sample->CTS_Offset;
			raf->trun_number = gf_list_count(traf->TrackRuns);
			raf->sample_number = trun->sample_count;
		}
	}
	return GF_OK;
}