static void get_info_for_all_streams (mpeg2ps_t *ps)
{
	u8 stream_ix, max_ix, av;
	mpeg2ps_stream_t *sptr;
	u8 *buffer;
	u32 buflen;
	file_seek_to(ps->fd, 0);
	for (av = 0; av < 2; av++) {
		if (av == 0) max_ix = ps->video_cnt;
		else max_ix = ps->audio_cnt;
		for (stream_ix = 0; stream_ix < max_ix; stream_ix++) {
			if (av == 0) sptr = ps->video_streams[stream_ix];
			else sptr = ps->audio_streams[stream_ix];
			sptr->m_fd = ps->fd;  
			clear_stream_buffer(sptr);
			if (mpeg2ps_stream_read_frame(sptr,
			                              &buffer,
			                              &buflen,
			                              0) == 0) {
				sptr->m_stream_id = 0;
				sptr->m_fd = FDNULL;
				continue;
			}
			get_info_from_frame(sptr, buffer, buflen);
			if (sptr->first_pes_has_dts == 0) {
				u32 frames_from_beg = 0;
				Bool have_frame;
				do {
					advance_frame(sptr);
					have_frame =
					    mpeg2ps_stream_read_frame(sptr, &buffer, &buflen, 0);
					frames_from_beg++;
				} while (have_frame &&
				         sptr->frame_ts.have_dts == 0 &&
				         sptr->frame_ts.have_pts == 0 &&
				         frames_from_beg < 1000);
				if (have_frame == 0 ||
				        (sptr->frame_ts.have_dts == 0 &&
				         sptr->frame_ts.have_pts == 0)) {
				} else {
					sptr->start_dts = sptr->frame_ts.have_dts ? sptr->frame_ts.dts :
					                  sptr->frame_ts.pts;
					if (sptr->is_video) {
						sptr->start_dts -= frames_from_beg * sptr->ticks_per_frame;
					} else {
						u64 conv;
						conv = sptr->samples_per_frame * 90000;
						conv /= (u64)sptr->freq;
						sptr->start_dts -= conv;
					}
				}
			}
			clear_stream_buffer(sptr);
			sptr->m_fd = FDNULL;
		}
	}
}