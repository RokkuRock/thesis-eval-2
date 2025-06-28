GF_Err text_box_read(GF_Box *s, GF_BitStream *bs)
{
	GF_Err e;
	u16 pSize;
	GF_TextSampleEntryBox *ptr = (GF_TextSampleEntryBox*)s;
	ISOM_DECREASE_SIZE(ptr, 51);
	e = gf_isom_base_sample_entry_read((GF_SampleEntryBox *)ptr, bs);
	if (e) return e;
	ptr->displayFlags = gf_bs_read_u32(bs);			 
	ptr->textJustification = gf_bs_read_u32(bs);	 
	gf_bs_read_data(bs, ptr->background_color, 6);	 
	gpp_read_box(bs, &ptr->default_box);			 
	gf_bs_read_data(bs, ptr->reserved1, 8);			 
	ptr->fontNumber = gf_bs_read_u16(bs);			 
	ptr->fontFace   = gf_bs_read_u16(bs);			 
	ptr->reserved2  = gf_bs_read_u8(bs);			 
	ptr->reserved3  = gf_bs_read_u16(bs);			 
	gf_bs_read_data(bs, ptr->foreground_color, 6);	 
	if (!ptr->size)
		return GF_OK;
	ISOM_DECREASE_SIZE(ptr, 1);
	pSize = gf_bs_read_u8(bs);  
	if (ptr->size < pSize) {
		u32 b_size = pSize;
		size_t i = 0;
		GF_LOG(GF_LOG_DEBUG, GF_LOG_CONTAINER, ("[iso file] text box doesn't use a Pascal string: trying to decode anyway.\n"));
		ptr->textName = (char*)gf_malloc((size_t)ptr->size + 1 + 1);
		if (!ptr->textName) return GF_OUT_OF_MEM;
		do {
			char c = (char)b_size;
			if (c == '\0') {
				break;
			} else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
				ptr->textName[i] = c;
			} else {
				gf_free(ptr->textName);
				ptr->textName = NULL;
				GF_LOG(GF_LOG_WARNING, GF_LOG_CONTAINER, ("[iso file] text box doesn't use a Pascal string and contains non-chars. Abort.\n"));
				return GF_ISOM_INVALID_FILE;
			}
			i++;
			if (!ptr->size)
				break;
			ptr->size--;
			b_size = gf_bs_read_u8(bs);
		} while (b_size);
		ptr->textName[i] = '\0';				 
		GF_LOG(GF_LOG_WARNING, GF_LOG_CONTAINER, ("[iso file] text box doesn't use a Pascal string: \"%s\" detected.\n", ptr->textName));
		return GF_OK;
	}
	if (pSize) {
		ptr->textName = (char*) gf_malloc(pSize+1 * sizeof(char));
		if (!ptr->textName) return GF_OUT_OF_MEM;
		if (gf_bs_read_data(bs, ptr->textName, pSize) != pSize) {
			gf_free(ptr->textName);
			ptr->textName = NULL;
			return GF_ISOM_INVALID_FILE;
		}
		ptr->textName[pSize] = '\0';				 
	}
	ISOM_DECREASE_SIZE(ptr, pSize);
	return gf_isom_box_array_read(s, bs);
}