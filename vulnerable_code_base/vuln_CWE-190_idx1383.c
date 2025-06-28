static INLINE OPJ_BOOL opj_tcd_init_tile(opj_tcd_t *p_tcd, OPJ_UINT32 p_tile_no, OPJ_BOOL isEncoder, OPJ_FLOAT32 fraction, OPJ_SIZE_T sizeof_block)
{
	OPJ_UINT32 (*l_gain_ptr)(OPJ_UINT32) = 00;
	OPJ_UINT32 compno, resno, bandno, precno, cblkno;
	opj_tcp_t * l_tcp = 00;
	opj_cp_t * l_cp = 00;
	opj_tcd_tile_t * l_tile = 00;
	opj_tccp_t *l_tccp = 00;
	opj_tcd_tilecomp_t *l_tilec = 00;
	opj_image_comp_t * l_image_comp = 00;
	opj_tcd_resolution_t *l_res = 00;
	opj_tcd_band_t *l_band = 00;
	opj_stepsize_t * l_step_size = 00;
	opj_tcd_precinct_t *l_current_precinct = 00;
	opj_image_t *l_image = 00;
	OPJ_UINT32 p,q;
	OPJ_UINT32 l_level_no;
	OPJ_UINT32 l_pdx, l_pdy;
	OPJ_UINT32 l_gain;
	OPJ_INT32 l_x0b, l_y0b;
	OPJ_INT32 l_tl_prc_x_start, l_tl_prc_y_start, l_br_prc_x_end, l_br_prc_y_end;
	OPJ_UINT32 l_nb_precincts;
	OPJ_UINT32 l_nb_precinct_size;
	OPJ_UINT32 l_nb_code_blocks;
	OPJ_UINT32 l_nb_code_blocks_size;
	OPJ_UINT32 l_data_size;
	l_cp = p_tcd->cp;
	l_tcp = &(l_cp->tcps[p_tile_no]);
	l_tile = p_tcd->tcd_image->tiles;
	l_tccp = l_tcp->tccps;
	l_tilec = l_tile->comps;
	l_image = p_tcd->image;
	l_image_comp = p_tcd->image->comps;
	p = p_tile_no % l_cp->tw;        
	q = p_tile_no / l_cp->tw;
	l_tile->x0 = opj_int_max((OPJ_INT32)(l_cp->tx0 + p * l_cp->tdx), (OPJ_INT32)l_image->x0);
	l_tile->y0 = opj_int_max((OPJ_INT32)(l_cp->ty0 + q * l_cp->tdy), (OPJ_INT32)l_image->y0);
	l_tile->x1 = opj_int_min((OPJ_INT32)(l_cp->tx0 + (p + 1) * l_cp->tdx), (OPJ_INT32)l_image->x1);
	l_tile->y1 = opj_int_min((OPJ_INT32)(l_cp->ty0 + (q + 1) * l_cp->tdy), (OPJ_INT32)l_image->y1);
	if (l_tccp->numresolutions == 0) {
		fprintf(stderr, "tiles require at least one resolution\n");
		return OPJ_FALSE;
	}
	for (compno = 0; compno < l_tile->numcomps; ++compno) {
		l_image_comp->resno_decoded = 0;
		l_tilec->x0 = opj_int_ceildiv(l_tile->x0, (OPJ_INT32)l_image_comp->dx);
		l_tilec->y0 = opj_int_ceildiv(l_tile->y0, (OPJ_INT32)l_image_comp->dy);
		l_tilec->x1 = opj_int_ceildiv(l_tile->x1, (OPJ_INT32)l_image_comp->dx);
		l_tilec->y1 = opj_int_ceildiv(l_tile->y1, (OPJ_INT32)l_image_comp->dy);
		l_data_size = (OPJ_UINT32)(l_tilec->x1 - l_tilec->x0);
		if ((((OPJ_UINT32)-1) / l_data_size) < (OPJ_UINT32)(l_tilec->y1 - l_tilec->y0)) {
			return OPJ_FALSE;
		}
		l_data_size = l_data_size * (OPJ_UINT32)(l_tilec->y1 - l_tilec->y0);
		if ((((OPJ_UINT32)-1) / (OPJ_UINT32)sizeof(OPJ_UINT32)) < l_data_size) {
			return OPJ_FALSE;
		}
		l_data_size = l_data_size * (OPJ_UINT32)sizeof(OPJ_UINT32);
		l_tilec->numresolutions = l_tccp->numresolutions;
		if (l_tccp->numresolutions < l_cp->m_specific_param.m_dec.m_reduce) {
			l_tilec->minimum_num_resolutions = 1;
		}
		else {
			l_tilec->minimum_num_resolutions = l_tccp->numresolutions - l_cp->m_specific_param.m_dec.m_reduce;
		}
		l_tilec->data_size_needed = l_data_size;
		if (p_tcd->m_is_decoder && !opj_alloc_tile_component_data(l_tilec)) {
			return OPJ_FALSE;
		}
		l_data_size = l_tilec->numresolutions * (OPJ_UINT32)sizeof(opj_tcd_resolution_t);
		if (l_tilec->resolutions == 00) {
			l_tilec->resolutions = (opj_tcd_resolution_t *) opj_malloc(l_data_size);
			if (! l_tilec->resolutions ) {
				return OPJ_FALSE;
			}
			l_tilec->resolutions_size = l_data_size;
			memset(l_tilec->resolutions,0,l_data_size);
		}
		else if (l_data_size > l_tilec->resolutions_size) {
			opj_tcd_resolution_t* new_resolutions = (opj_tcd_resolution_t *) opj_realloc(l_tilec->resolutions, l_data_size);
			if (! new_resolutions) {
				fprintf(stderr, "Not enough memory to tile resolutions\n");
				opj_free(l_tilec->resolutions);
				l_tilec->resolutions = NULL;
				l_tilec->resolutions_size = 0;
				return OPJ_FALSE;
			}
			l_tilec->resolutions = new_resolutions;
			memset(((OPJ_BYTE*) l_tilec->resolutions)+l_tilec->resolutions_size,0,l_data_size - l_tilec->resolutions_size);
			l_tilec->resolutions_size = l_data_size;
		}
		l_level_no = l_tilec->numresolutions - 1;
		l_res = l_tilec->resolutions;
		l_step_size = l_tccp->stepsizes;
		if (l_tccp->qmfbid == 0) {
			l_gain_ptr = &opj_dwt_getgain_real;
		}
		else {
			l_gain_ptr  = &opj_dwt_getgain;
		}
		for (resno = 0; resno < l_tilec->numresolutions; ++resno) {
			OPJ_INT32 tlcbgxstart, tlcbgystart  ;
			OPJ_UINT32 cbgwidthexpn, cbgheightexpn;
			OPJ_UINT32 cblkwidthexpn, cblkheightexpn;
			l_res->x0 = opj_int_ceildivpow2(l_tilec->x0, (OPJ_INT32)l_level_no);
			l_res->y0 = opj_int_ceildivpow2(l_tilec->y0, (OPJ_INT32)l_level_no);
			l_res->x1 = opj_int_ceildivpow2(l_tilec->x1, (OPJ_INT32)l_level_no);
			l_res->y1 = opj_int_ceildivpow2(l_tilec->y1, (OPJ_INT32)l_level_no);
			l_pdx = l_tccp->prcw[resno];
			l_pdy = l_tccp->prch[resno];
			l_tl_prc_x_start = opj_int_floordivpow2(l_res->x0, (OPJ_INT32)l_pdx) << l_pdx;
			l_tl_prc_y_start = opj_int_floordivpow2(l_res->y0, (OPJ_INT32)l_pdy) << l_pdy;
			l_br_prc_x_end = opj_int_ceildivpow2(l_res->x1, (OPJ_INT32)l_pdx) << l_pdx;
			l_br_prc_y_end = opj_int_ceildivpow2(l_res->y1, (OPJ_INT32)l_pdy) << l_pdy;
			l_res->pw = (l_res->x0 == l_res->x1) ? 0 : (OPJ_UINT32)((l_br_prc_x_end - l_tl_prc_x_start) >> l_pdx);
			l_res->ph = (l_res->y0 == l_res->y1) ? 0 : (OPJ_UINT32)((l_br_prc_y_end - l_tl_prc_y_start) >> l_pdy);
			l_nb_precincts = l_res->pw * l_res->ph;
			l_nb_precinct_size = l_nb_precincts * (OPJ_UINT32)sizeof(opj_tcd_precinct_t);
			if (resno == 0) {
				tlcbgxstart = l_tl_prc_x_start;
				tlcbgystart = l_tl_prc_y_start;
				cbgwidthexpn = l_pdx;
				cbgheightexpn = l_pdy;
				l_res->numbands = 1;
			}
			else {
				tlcbgxstart = opj_int_ceildivpow2(l_tl_prc_x_start, 1);
				tlcbgystart = opj_int_ceildivpow2(l_tl_prc_y_start, 1);
				cbgwidthexpn = l_pdx - 1;
				cbgheightexpn = l_pdy - 1;
				l_res->numbands = 3;
			}
			cblkwidthexpn = opj_uint_min(l_tccp->cblkw, cbgwidthexpn);
			cblkheightexpn = opj_uint_min(l_tccp->cblkh, cbgheightexpn);
			l_band = l_res->bands;
			for (bandno = 0; bandno < l_res->numbands; ++bandno) {
				OPJ_INT32 numbps;
				if (resno == 0) {
					l_band->bandno = 0 ;
					l_band->x0 = opj_int_ceildivpow2(l_tilec->x0, (OPJ_INT32)l_level_no);
					l_band->y0 = opj_int_ceildivpow2(l_tilec->y0, (OPJ_INT32)l_level_no);
					l_band->x1 = opj_int_ceildivpow2(l_tilec->x1, (OPJ_INT32)l_level_no);
					l_band->y1 = opj_int_ceildivpow2(l_tilec->y1, (OPJ_INT32)l_level_no);
				}
				else {
					l_band->bandno = bandno + 1;
					l_x0b = l_band->bandno&1;
					l_y0b = (OPJ_INT32)((l_band->bandno)>>1);
					l_band->x0 = opj_int_ceildivpow2(l_tilec->x0 - (1 << l_level_no) * l_x0b, (OPJ_INT32)(l_level_no + 1));
					l_band->y0 = opj_int_ceildivpow2(l_tilec->y0 - (1 << l_level_no) * l_y0b, (OPJ_INT32)(l_level_no + 1));
					l_band->x1 = opj_int_ceildivpow2(l_tilec->x1 - (1 << l_level_no) * l_x0b, (OPJ_INT32)(l_level_no + 1));
					l_band->y1 = opj_int_ceildivpow2(l_tilec->y1 - (1 << l_level_no) * l_y0b, (OPJ_INT32)(l_level_no + 1));
				}
				l_gain = (*l_gain_ptr) (l_band->bandno);
				numbps = (OPJ_INT32)(l_image_comp->prec + l_gain);
				l_band->stepsize = (OPJ_FLOAT32)(((1.0 + l_step_size->mant / 2048.0) * pow(2.0, (OPJ_INT32) (numbps - l_step_size->expn)))) * fraction;
				l_band->numbps = l_step_size->expn + (OPJ_INT32)l_tccp->numgbits - 1;       
				if (! l_band->precincts) {
					l_band->precincts = (opj_tcd_precinct_t *) opj_malloc(   l_nb_precinct_size);
					if (! l_band->precincts) {
						return OPJ_FALSE;
					}
					memset(l_band->precincts,0,l_nb_precinct_size);
					l_band->precincts_data_size = l_nb_precinct_size;
				}
				else if (l_band->precincts_data_size < l_nb_precinct_size) {
					opj_tcd_precinct_t * new_precincts = (opj_tcd_precinct_t *) opj_realloc(l_band->precincts,  l_nb_precinct_size);
					if (! new_precincts) {
						fprintf(stderr, "Not enough memory to handle band precints\n");
						opj_free(l_band->precincts);
						l_band->precincts = NULL;
						l_band->precincts_data_size = 0;
						return OPJ_FALSE;
					}
					l_band->precincts = new_precincts;
					memset(((OPJ_BYTE *) l_band->precincts) + l_band->precincts_data_size,0,l_nb_precinct_size - l_band->precincts_data_size);
					l_band->precincts_data_size = l_nb_precinct_size;
				}
				l_current_precinct = l_band->precincts;
				for (precno = 0; precno < l_nb_precincts; ++precno) {
					OPJ_INT32 tlcblkxstart, tlcblkystart, brcblkxend, brcblkyend;
					OPJ_INT32 cbgxstart = tlcbgxstart + (OPJ_INT32)(precno % l_res->pw) * (1 << cbgwidthexpn);
					OPJ_INT32 cbgystart = tlcbgystart + (OPJ_INT32)(precno / l_res->pw) * (1 << cbgheightexpn);
					OPJ_INT32 cbgxend = cbgxstart + (1 << cbgwidthexpn);
					OPJ_INT32 cbgyend = cbgystart + (1 << cbgheightexpn);
					l_current_precinct->x0 = opj_int_max(cbgxstart, l_band->x0);
					l_current_precinct->y0 = opj_int_max(cbgystart, l_band->y0);
					l_current_precinct->x1 = opj_int_min(cbgxend, l_band->x1);
					l_current_precinct->y1 = opj_int_min(cbgyend, l_band->y1);
					tlcblkxstart = opj_int_floordivpow2(l_current_precinct->x0, (OPJ_INT32)cblkwidthexpn) << cblkwidthexpn;
					tlcblkystart = opj_int_floordivpow2(l_current_precinct->y0, (OPJ_INT32)cblkheightexpn) << cblkheightexpn;
					brcblkxend = opj_int_ceildivpow2(l_current_precinct->x1, (OPJ_INT32)cblkwidthexpn) << cblkwidthexpn;
					brcblkyend = opj_int_ceildivpow2(l_current_precinct->y1, (OPJ_INT32)cblkheightexpn) << cblkheightexpn;
					l_current_precinct->cw = (OPJ_UINT32)((brcblkxend - tlcblkxstart) >> cblkwidthexpn);
					l_current_precinct->ch = (OPJ_UINT32)((brcblkyend - tlcblkystart) >> cblkheightexpn);
					l_nb_code_blocks = l_current_precinct->cw * l_current_precinct->ch;
					l_nb_code_blocks_size = l_nb_code_blocks * (OPJ_UINT32)sizeof_block;
					if (! l_current_precinct->cblks.blocks) {
						l_current_precinct->cblks.blocks = opj_malloc(l_nb_code_blocks_size);
						if (! l_current_precinct->cblks.blocks ) {
							return OPJ_FALSE;
						}
						memset(l_current_precinct->cblks.blocks,0,l_nb_code_blocks_size);
						l_current_precinct->block_size = l_nb_code_blocks_size;
					}
					else if (l_nb_code_blocks_size > l_current_precinct->block_size) {
						void *new_blocks = opj_realloc(l_current_precinct->cblks.blocks, l_nb_code_blocks_size);
						if (! new_blocks) {
							opj_free(l_current_precinct->cblks.blocks);
							l_current_precinct->cblks.blocks = NULL;
							l_current_precinct->block_size = 0;
							fprintf(stderr, "Not enough memory for current precinct codeblock element\n");
							return OPJ_FALSE;
						}
						l_current_precinct->cblks.blocks = new_blocks;
						memset(((OPJ_BYTE *) l_current_precinct->cblks.blocks) + l_current_precinct->block_size
									 ,0
									 ,l_nb_code_blocks_size - l_current_precinct->block_size);
						l_current_precinct->block_size = l_nb_code_blocks_size;
					}
					if (! l_current_precinct->incltree) {
						l_current_precinct->incltree = opj_tgt_create(l_current_precinct->cw,
																													l_current_precinct->ch);
					}
					else{
						l_current_precinct->incltree = opj_tgt_init(l_current_precinct->incltree,
																												l_current_precinct->cw,
																												l_current_precinct->ch);
					}
					if (! l_current_precinct->incltree)     {
						fprintf(stderr, "WARNING: No incltree created.\n");
					}
					if (! l_current_precinct->imsbtree) {
						l_current_precinct->imsbtree = opj_tgt_create(
																													l_current_precinct->cw,
																													l_current_precinct->ch);
					}
					else {
						l_current_precinct->imsbtree = opj_tgt_init(
																												l_current_precinct->imsbtree,
																												l_current_precinct->cw,
																												l_current_precinct->ch);
					}
					if (! l_current_precinct->imsbtree) {
						fprintf(stderr, "WARNING: No imsbtree created.\n");
					}
					for (cblkno = 0; cblkno < l_nb_code_blocks; ++cblkno) {
						OPJ_INT32 cblkxstart = tlcblkxstart + (OPJ_INT32)(cblkno % l_current_precinct->cw) * (1 << cblkwidthexpn);
						OPJ_INT32 cblkystart = tlcblkystart + (OPJ_INT32)(cblkno / l_current_precinct->cw) * (1 << cblkheightexpn);
						OPJ_INT32 cblkxend = cblkxstart + (1 << cblkwidthexpn);
						OPJ_INT32 cblkyend = cblkystart + (1 << cblkheightexpn);
						if (isEncoder) {
							opj_tcd_cblk_enc_t* l_code_block = l_current_precinct->cblks.enc + cblkno;
							if (! opj_tcd_code_block_enc_allocate(l_code_block)) {
								return OPJ_FALSE;
							}
							l_code_block->x0 = opj_int_max(cblkxstart, l_current_precinct->x0);
							l_code_block->y0 = opj_int_max(cblkystart, l_current_precinct->y0);
							l_code_block->x1 = opj_int_min(cblkxend, l_current_precinct->x1);
							l_code_block->y1 = opj_int_min(cblkyend, l_current_precinct->y1);
							if (! opj_tcd_code_block_enc_allocate_data(l_code_block)) {
								return OPJ_FALSE;
							}
						} else {
							opj_tcd_cblk_dec_t* l_code_block = l_current_precinct->cblks.dec + cblkno;
							if (! opj_tcd_code_block_dec_allocate(l_code_block)) {
								return OPJ_FALSE;
							}
							l_code_block->x0 = opj_int_max(cblkxstart, l_current_precinct->x0);
							l_code_block->y0 = opj_int_max(cblkystart, l_current_precinct->y0);
							l_code_block->x1 = opj_int_min(cblkxend, l_current_precinct->x1);
							l_code_block->y1 = opj_int_min(cblkyend, l_current_precinct->y1);
						}
					}
					++l_current_precinct;
				}  
				++l_band;
				++l_step_size;
			}  
			++l_res;
			--l_level_no;
		}  
		++l_tccp;
		++l_tilec;
		++l_image_comp;
	}  
	return OPJ_TRUE;
}