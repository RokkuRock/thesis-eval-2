GF_Err dac3_box_write(GF_Box *s, GF_BitStream *bs)
{
	GF_Err e;
	GF_AC3ConfigBox *ptr = (GF_AC3ConfigBox *)s;
	if (ptr->cfg.is_ec3) s->type = GF_ISOM_BOX_TYPE_DEC3;
	e = gf_isom_box_write_header(s, bs);
	if (ptr->cfg.is_ec3) s->type = GF_ISOM_BOX_TYPE_DAC3;
	if (e) return e;
	e = gf_odf_ac3_cfg_write_bs(&ptr->cfg, bs);
	if (e) return e;
	if (ptr->cfg.atmos_ec3_ext || ptr->cfg.complexity_index_type) {
		gf_bs_write_int(bs, 0, 7);
		gf_bs_write_int(bs, ptr->cfg.atmos_ec3_ext, 1);
		gf_bs_write_u8(bs, ptr->cfg.complexity_index_type);
	}
	return GF_OK;
}