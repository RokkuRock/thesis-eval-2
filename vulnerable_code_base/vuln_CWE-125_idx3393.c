exif_mnote_data_fuji_load (ExifMnoteData *en,
	const unsigned char *buf, unsigned int buf_size)
{
	ExifMnoteDataFuji *n = (ExifMnoteDataFuji*) en;
	ExifLong c;
	size_t i, tcount, o, datao;
	if (!n || !buf || !buf_size) {
		exif_log (en->log, EXIF_LOG_CODE_CORRUPT_DATA,
			  "ExifMnoteDataFuji", "Short MakerNote");
		return;
	}
	datao = 6 + n->offset;
	if ((datao + 12 < datao) || (datao + 12 < 12) || (datao + 12 > buf_size)) {
		exif_log (en->log, EXIF_LOG_CODE_CORRUPT_DATA,
			  "ExifMnoteDataFuji", "Short MakerNote");
		return;
	}
	n->order = EXIF_BYTE_ORDER_INTEL;
	datao += exif_get_long (buf + datao + 8, EXIF_BYTE_ORDER_INTEL);
	if ((datao + 2 < datao) || (datao + 2 < 2) ||
	    (datao + 2 > buf_size)) {
		exif_log (en->log, EXIF_LOG_CODE_CORRUPT_DATA,
			  "ExifMnoteDataFuji", "Short MakerNote");
		return;
	}
	c = exif_get_short (buf + datao, EXIF_BYTE_ORDER_INTEL);
	datao += 2;
	exif_mnote_data_fuji_clear (n);
	n->entries = exif_mem_alloc (en->mem, sizeof (MnoteFujiEntry) * c);
	if (!n->entries) {
		EXIF_LOG_NO_MEMORY(en->log, "ExifMnoteDataFuji", sizeof (MnoteFujiEntry) * c);
		return;
	}
	tcount = 0;
	for (i = c, o = datao; i; --i, o += 12) {
		size_t s;
		if ((o + 12 < o) || (o + 12 < 12) || (o + 12 > buf_size)) {
			exif_log (en->log, EXIF_LOG_CODE_CORRUPT_DATA,
				  "ExifMnoteDataFuji", "Short MakerNote");
			break;
		}
		n->entries[tcount].tag        = exif_get_short (buf + o, n->order);
		n->entries[tcount].format     = exif_get_short (buf + o + 2, n->order);
		n->entries[tcount].components = exif_get_long (buf + o + 4, n->order);
		n->entries[tcount].order      = n->order;
		exif_log (en->log, EXIF_LOG_CODE_DEBUG, "ExifMnoteDataFuji",
			  "Loading entry 0x%x ('%s')...", n->entries[tcount].tag,
			  mnote_fuji_tag_get_name (n->entries[tcount].tag));
		s = exif_format_get_size (n->entries[tcount].format) * n->entries[tcount].components;
		n->entries[tcount].size = s;
		if (s) {
			size_t dataofs = o + 8;
			if (s > 4)
				dataofs = exif_get_long (buf + dataofs, n->order) + 6 + n->offset;
			if ((dataofs + s < dataofs) || (dataofs + s < s) ||
				(dataofs + s >= buf_size)) {
				exif_log (en->log, EXIF_LOG_CODE_CORRUPT_DATA,
						  "ExifMnoteDataFuji", "Tag data past end of "
					  "buffer (%u >= %u)", (unsigned)(dataofs + s), buf_size);
				continue;
			}
			n->entries[tcount].data = exif_mem_alloc (en->mem, s);
			if (!n->entries[tcount].data) {
				EXIF_LOG_NO_MEMORY(en->log, "ExifMnoteDataFuji", s);
				continue;
			}
			memcpy (n->entries[tcount].data, buf + dataofs, s);
		}
		++tcount;
	}
	n->count = tcount;
}