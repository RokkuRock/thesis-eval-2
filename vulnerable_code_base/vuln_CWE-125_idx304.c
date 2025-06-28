GF_Err vobsub_read_idx(FILE *file, vobsub_file *vobsub, s32 *version)
{
	char  strbuf[256];
	char *str, *pos, *entry;
	s32   line, id =-1, delay = 0;
	Bool  error = 0;
	for (line = 0; !error && gf_fgets(strbuf, sizeof(strbuf), file); line++)
	{
		str = strtrim(strbuf);
		if (line == 0)
		{
			char *buf = "VobSub index file, v";
			pos = strstr(str, buf);
			if (pos == NULL || sscanf(pos + strlen(buf), "%d", version) != 1 || *version > VOBSUBIDXVER)
			{
				error = 1;
				continue;
			}
		}
		else if (strlen(str) == 0)
		{
			continue;
		}
		else if (str[0] == '#')
		{
			continue;
		}
		pos = strchr(str, ':');
		if (pos == NULL || pos == str)
		{
			continue;
		}
		entry = str;
		*pos  = '\0';
		str = strtrim(pos + 1);
		if (strlen(str) == 0)
		{
			continue;
		}
		if (stricmp(entry, "size") == 0)
		{
			s32 w, h;
			if (sscanf(str, "%dx%d", &w, &h) != 2)
			{
				error = 1;
			}
			vobsub->width  = w;
			vobsub->height = h;
		}
		else if (stricmp(entry, "palette") == 0)
		{
			s32 c;
			u8  palette[16][4];
			if (sscanf(str, "%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x,%x",
			           (u32 *) &palette[0], (u32 *) &palette[1], (u32 *) &palette[2], (u32 *) &palette[3],
			           (u32 *) &palette[4], (u32 *) &palette[5], (u32 *) &palette[6], (u32 *) &palette[7],
			           (u32 *) &palette[8], (u32 *) &palette[9], (u32 *) &palette[10], (u32 *) &palette[11],
			           (u32 *) &palette[12],(u32 *) &palette[13],(u32 *) &palette[14], (u32 *) &palette[15]) != 16)
			{
				error = 1;
				continue;
			}
			for (c = 0; c < 16; c++)
			{
				u8 r, g, b;
				r = palette[c][2];
				g = palette[c][1];
				b = palette[c][0];
				vobsub->palette[c][0] = 0;
				vobsub->palette[c][1] = (( 66 * r + 129 * g +  25 * b + 128 +  4096) >> 8) & 0xff;
				vobsub->palette[c][2] = ((112 * r -  94 * g -  18 * b + 128 + 32768) >> 8) & 0xff;
				vobsub->palette[c][3] = ((-38 * r -  74 * g + 112 * b + 128 + 32768) >> 8) & 0xff;
			}
		}
		else if (stricmp(entry, "id") == 0)
		{
			char *buf = "index:";
			s32   lang_id;
			strlwr(str);
			lang_id = ((str[0] & 0xff) << 8) | (str[1] & 0xff);
			pos = strstr(str, buf);
			if (pos == NULL)
			{
				error = 1;
				continue;
			}
			if (sscanf(pos + strlen(buf), "%d", &id) != 1 || id < 0 || id >= 32)
			{
				error = 1;
				continue;
			}
			vobsub->langs[id].id   = lang_id;
			vobsub->langs[id].name = lang_table[vobsub_lang_name((u16)lang_id)].lang;
			vobsub->langs[id].idx = id;
			vobsub->langs[id].subpos = gf_list_new();
			if (vobsub->langs[id].subpos == NULL)
			{
				error = 1;
				continue;
			}
			delay = 0;
			vobsub->num_langs++;
		}
		else if (id >= 0 && stricmp(entry, "delay") == 0)
		{
			s32  hh, mm, ss, ms;
			char c;
			s32  sign = (str[0] == '-') ? -1 : 1;
			pos = str;
			while (*pos == '-' || *pos == '+') pos++;
			if (sscanf(pos, "%d%c%d%c%d%c%d", &hh, &c, &mm, &c, &ss, &c, &ms) != 7)
			{
				error = 1;
				continue;
			}
			delay += (hh*60*60*1000 + mm*60*1000 + ss*1000 + ms) * sign;
		}
		else if (id >= 0 && stricmp(entry, "timestamp") == 0)
		{
			vobsub_pos *vspos;
			s32         sign;
			char        c;
			s32         hh, mm, ss, ms;
			char       *buf = "filepos:";
			vspos = (vobsub_pos*)gf_calloc(1, sizeof(vobsub_pos));
			if (vspos == NULL) {
				error = 1;
				continue;
			}
			sign = (str[0] == '-') ? -1 : 1;
			while (*str == '-' || *str == '+') str++;
			if (sscanf(str, "%d%c%d%c%d%c%d", &hh, &c, &mm, &c, &ss, &c, &ms) != 7)
			{
				gf_free(vspos);
				error = 1;
				continue;
			}
			vspos->start = (((hh*60 + mm)*60 + ss)*1000 + ms) * sign + delay;
			pos = strstr(str, buf);
			if (pos == NULL)
			{
				gf_free(vspos);
				error = 1;
				continue;
			}
			if (sscanf(pos + strlen(buf), LLX, &vspos->filepos) != 1)
			{
				gf_free(vspos);
				error = 1;
				continue;
			}
			if (delay < 0 && gf_list_count(vobsub->langs[id].subpos) > 0)
			{
				vobsub_pos *vspos_next;
				vspos_next = (vobsub_pos*)gf_list_get(vobsub->langs[id].subpos, gf_list_count(vobsub->langs[id].subpos) - 1);
				if (vspos->start < vspos_next->start)
				{
					delay += (s32)(vspos_next->start - vspos->start);
					vspos->start = vspos_next->start;
				}
			}
			if (gf_list_add(vobsub->langs[id].subpos, vspos) != GF_OK)
			{
				gf_free(vspos);
				error = 1;
				continue;
			}
		}
	}
	return error ? GF_CORRUPTED_DATA : GF_OK;
}