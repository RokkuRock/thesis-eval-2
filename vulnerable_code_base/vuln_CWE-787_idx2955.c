int fmt_mtm_load_song(song_t *song, slurp_t *fp, unsigned int lflags)
{
	uint8_t b[192];
	uint8_t nchan, nord, npat, nsmp;
	uint16_t ntrk, comment_len;
	int n, pat, chan, smp, rows, todo = 0;
	song_note_t *note;
	uint16_t tmp;
	uint32_t tmplong;
	song_note_t **trackdata, *tracknote;
	song_sample_t *sample;
	slurp_read(fp, b, 3);
	if (memcmp(b, "MTM", 3) != 0)
		return LOAD_UNSUPPORTED;
	n = slurp_getc(fp);
	sprintf(song->tracker_id, "MultiTracker %d.%d", n >> 4, n & 0xf);
	slurp_read(fp, song->title, 20);
	song->title[20] = 0;
	slurp_read(fp, &ntrk, 2);
	ntrk = bswapLE16(ntrk);
	npat = slurp_getc(fp);
	nord = slurp_getc(fp) + 1;
	slurp_read(fp, &comment_len, 2);
	comment_len = bswapLE16(comment_len);
	nsmp = slurp_getc(fp);
	slurp_getc(fp);  
	rows = slurp_getc(fp);  
	if (rows != 64)
		todo |= 64;
	rows = MIN(rows, 64);
	nchan = slurp_getc(fp);
	if (slurp_eof(fp)) {
		return LOAD_FORMAT_ERROR;
	}
	for (n = 0; n < 32; n++) {
		int pan = slurp_getc(fp) & 0xf;
		pan = SHORT_PANNING(pan);
		pan *= 4;  
		song->channels[n].panning = pan;
	}
	for (n = nchan; n < MAX_CHANNELS; n++)
		song->channels[n].flags = CHN_MUTE;
	if (nsmp > MAX_SAMPLES) {
		log_appendf(4, " Warning: Too many samples");
	}
	for (n = 1, sample = song->samples + 1; n <= nsmp; n++, sample++) {
		if (n > MAX_SAMPLES) {
			slurp_seek(fp, 37, SEEK_CUR);
			continue;
		}
		char name[23];
		slurp_read(fp, name, 22);
		name[22] = '\0';
		strcpy(sample->name, name);
		slurp_read(fp, &tmplong, 4);
		sample->length = bswapLE32(tmplong);
		slurp_read(fp, &tmplong, 4);
		sample->loop_start = bswapLE32(tmplong);
		slurp_read(fp, &tmplong, 4);
		sample->loop_end = bswapLE32(tmplong);
		if ((sample->loop_end - sample->loop_start) > 2) {
			sample->flags |= CHN_LOOP;
		} else {
			sample->loop_start = 0;
			sample->loop_end = 0;
		}
		song->samples[n].c5speed = MOD_FINETUNE(slurp_getc(fp));
		sample->volume = slurp_getc(fp);
		sample->volume *= 4;  
		sample->global_volume = 64;
		if (slurp_getc(fp) & 1) {
			todo |= 16;
			sample->flags |= CHN_16BIT;
			sample->length >>= 1;
			sample->loop_start >>= 1;
			sample->loop_end >>= 1;
		}
		song->samples[n].vib_type = 0;
		song->samples[n].vib_rate = 0;
		song->samples[n].vib_depth = 0;
		song->samples[n].vib_speed = 0;
	}
	slurp_read(fp, song->orderlist, 128);
	memset(song->orderlist + nord, ORDER_LAST, MAX_ORDERS - nord);
	trackdata = mem_calloc(ntrk, sizeof(song_note_t *));
	for (n = 0; n < ntrk; n++) {
		slurp_read(fp, b, 3 * rows);
		trackdata[n] = mem_calloc(rows, sizeof(song_note_t));
		mtm_unpack_track(b, trackdata[n], rows);
	}
	if (npat >= MAX_PATTERNS) {
		log_appendf(4, " Warning: Too many patterns");
	}
	for (pat = 0; pat <= npat; pat++) {
		if (pat >= MAX_PATTERNS) {
			slurp_seek(fp, 64, SEEK_CUR);
			continue;
		}
		song->patterns[pat] = csf_allocate_pattern(MAX(rows, 32));
		song->pattern_size[pat] = song->pattern_alloc_size[pat] = 64;
		tracknote = trackdata[n];
		for (chan = 0; chan < 32; chan++) {
			slurp_read(fp, &tmp, 2);
			tmp = bswapLE16(tmp);
			if (tmp == 0) {
				continue;
			} else if (tmp > ntrk) {
				for (n = 0; n < ntrk; n++)
					free(trackdata[n]);
				free(trackdata);
				return LOAD_FORMAT_ERROR;
			}
			note = song->patterns[pat] + chan;
			tracknote = trackdata[tmp - 1];
			for (n = 0; n < rows; n++, tracknote++, note += MAX_CHANNELS)
				*note = *tracknote;
		}
		if (rows < 32) {
			note = song->patterns[pat] + 64 * (rows - 1);
			while (note->effect || note->param)
				note++;
			note->effect = FX_PATTERNBREAK;
		}
	}
	for (n = 0; n < ntrk; n++)
		free(trackdata[n]);
	free(trackdata);
	read_lined_message(song->message, fp, comment_len, 40);
	if (!(lflags & LOAD_NOSAMPLES)) {
		for (smp = 1; smp <= nsmp && smp <= MAX_SAMPLES; smp++) {
			uint32_t ssize;
			if (song->samples[smp].length == 0)
				continue;
			ssize = csf_read_sample(song->samples + smp,
				(SF_LE | SF_PCMU | SF_M
				 | ((song->samples[smp].flags & CHN_16BIT) ? SF_16 : SF_8)),
				fp->data + fp->pos, fp->length - fp->pos);
			slurp_seek(fp, ssize, SEEK_CUR);
		}
	}
	song->flags = SONG_ITOLDEFFECTS | SONG_COMPATGXX;
	if (todo & 64)
		log_appendf(2, " TODO: test this file with other players (beats per track != 64)");
	if (todo & 16)
		log_appendf(2, " TODO: double check 16 bit sample loading");
	return LOAD_SUCCESS;
}