TIFFReadCustomDirectory(TIFF* tif, toff_t diroff,
			const TIFFFieldInfo info[], size_t n)
{
	static const char module[] = "TIFFReadCustomDirectory";
	TIFFDirectory* td = &tif->tif_dir;
	TIFFDirEntry *dp, *dir = NULL;
	const TIFFFieldInfo* fip;
	size_t fix;
	uint16 i, dircount;
	_TIFFSetupFieldInfo(tif, info, n);
	dircount = TIFFFetchDirectory(tif, diroff, &dir, NULL);
	if (!dircount) {
		TIFFErrorExt(tif->tif_clientdata, module,
			"%s: Failed to read custom directory at offset %u",
			     tif->tif_name, diroff);
		return 0;
	}
	TIFFFreeDirectory(tif);
        _TIFFmemset(&tif->tif_dir, 0, sizeof(TIFFDirectory));
	fix = 0;
	for (dp = dir, i = dircount; i > 0; i--, dp++) {
		if (tif->tif_flags & TIFF_SWAB) {
			TIFFSwabArrayOfShort(&dp->tdir_tag, 2);
			TIFFSwabArrayOfLong(&dp->tdir_count, 2);
		}
		if (fix >= tif->tif_nfields || dp->tdir_tag == IGNORE)
			continue;
		while (fix < tif->tif_nfields &&
		       tif->tif_fieldinfo[fix]->field_tag < dp->tdir_tag)
			fix++;
		if (fix >= tif->tif_nfields ||
		    tif->tif_fieldinfo[fix]->field_tag != dp->tdir_tag) {
			TIFFWarningExt(tif->tif_clientdata, module,
                        "%s: unknown field with tag %d (0x%x) encountered",
				    tif->tif_name, dp->tdir_tag, dp->tdir_tag);
			if (!_TIFFMergeFieldInfo(tif,
						 _TIFFCreateAnonFieldInfo(tif,
						 dp->tdir_tag,
						 (TIFFDataType) dp->tdir_type),
						 1))
			{
				TIFFWarningExt(tif->tif_clientdata, module,
			"Registering anonymous field with tag %d (0x%x) failed",
						dp->tdir_tag, dp->tdir_tag);
				goto ignore;
			}
			fix = 0;
			while (fix < tif->tif_nfields &&
			       tif->tif_fieldinfo[fix]->field_tag < dp->tdir_tag)
				fix++;
		}
		if (tif->tif_fieldinfo[fix]->field_bit == FIELD_IGNORE) {
	ignore:
			dp->tdir_tag = IGNORE;
			continue;
		}
		fip = tif->tif_fieldinfo[fix];
		while (dp->tdir_type != (unsigned short) fip->field_type
                       && fix < tif->tif_nfields) {
			if (fip->field_type == TIFF_ANY)	 
				break;
                        fip = tif->tif_fieldinfo[++fix];
			if (fix >= tif->tif_nfields ||
			    fip->field_tag != dp->tdir_tag) {
				TIFFWarningExt(tif->tif_clientdata, module,
			"%s: wrong data type %d for \"%s\"; tag ignored",
					    tif->tif_name, dp->tdir_type,
					    tif->tif_fieldinfo[fix-1]->field_name);
				goto ignore;
			}
		}
		if (fip->field_readcount != TIFF_VARIABLE
		    && fip->field_readcount != TIFF_VARIABLE2) {
			uint32 expected = (fip->field_readcount == TIFF_SPP) ?
			    (uint32) td->td_samplesperpixel :
			    (uint32) fip->field_readcount;
			if (!CheckDirCount(tif, dp, expected))
				goto ignore;
		}
		switch (dp->tdir_tag) {
			case EXIFTAG_SUBJECTDISTANCE:
				(void) TIFFFetchSubjectDistance(tif, dp);
				break;
			default:
				(void) TIFFFetchNormalTag(tif, dp);
				break;
		}
	}
	if (dir)
		_TIFFfree(dir);
	return 1;
}