jpeg_skip_scanlines(j_decompress_ptr cinfo, JDIMENSION num_lines)
{
  my_main_ptr main_ptr = (my_main_ptr)cinfo->main;
  my_coef_ptr coef = (my_coef_ptr)cinfo->coef;
  my_upsample_ptr upsample = (my_upsample_ptr)cinfo->upsample;
  JDIMENSION i, x;
  int y;
  JDIMENSION lines_per_iMCU_row, lines_left_in_iMCU_row, lines_after_iMCU_row;
  JDIMENSION lines_to_skip, lines_to_read;
  if (cinfo->global_state != DSTATE_SCANNING)
    ERREXIT1(cinfo, JERR_BAD_STATE, cinfo->global_state);
  if (cinfo->output_scanline + num_lines >= cinfo->output_height) {
    cinfo->output_scanline = cinfo->output_height;
    (*cinfo->inputctl->finish_input_pass) (cinfo);
    cinfo->inputctl->eoi_reached = TRUE;
    return cinfo->output_height - cinfo->output_scanline;
  }
  if (num_lines == 0)
    return 0;
  lines_per_iMCU_row = cinfo->_min_DCT_scaled_size * cinfo->max_v_samp_factor;
  lines_left_in_iMCU_row =
    (lines_per_iMCU_row - (cinfo->output_scanline % lines_per_iMCU_row)) %
    lines_per_iMCU_row;
  lines_after_iMCU_row = num_lines - lines_left_in_iMCU_row;
  if (cinfo->upsample->need_context_rows) {
    if ((num_lines < lines_left_in_iMCU_row + 1) ||
        (lines_left_in_iMCU_row <= 1 && main_ptr->buffer_full &&
         lines_after_iMCU_row < lines_per_iMCU_row + 1)) {
      read_and_discard_scanlines(cinfo, num_lines);
      return num_lines;
    }
    if (lines_left_in_iMCU_row <= 1 && main_ptr->buffer_full) {
      cinfo->output_scanline += lines_left_in_iMCU_row + lines_per_iMCU_row;
      lines_after_iMCU_row -= lines_per_iMCU_row;
    } else {
      cinfo->output_scanline += lines_left_in_iMCU_row;
    }
    if (main_ptr->iMCU_row_ctr == 0 ||
        (main_ptr->iMCU_row_ctr == 1 && lines_left_in_iMCU_row > 2))
      set_wraparound_pointers(cinfo);
    main_ptr->buffer_full = FALSE;
    main_ptr->rowgroup_ctr = 0;
    main_ptr->context_state = CTX_PREPARE_FOR_IMCU;
    upsample->next_row_out = cinfo->max_v_samp_factor;
    upsample->rows_to_go = cinfo->output_height - cinfo->output_scanline;
  }
  else {
    if (num_lines < lines_left_in_iMCU_row) {
      increment_simple_rowgroup_ctr(cinfo, num_lines);
      return num_lines;
    } else {
      cinfo->output_scanline += lines_left_in_iMCU_row;
      main_ptr->buffer_full = FALSE;
      main_ptr->rowgroup_ctr = 0;
      upsample->next_row_out = cinfo->max_v_samp_factor;
      upsample->rows_to_go = cinfo->output_height - cinfo->output_scanline;
    }
  }
  if (cinfo->upsample->need_context_rows)
    lines_to_skip = ((lines_after_iMCU_row - 1) / lines_per_iMCU_row) *
                    lines_per_iMCU_row;
  else
    lines_to_skip = (lines_after_iMCU_row / lines_per_iMCU_row) *
                    lines_per_iMCU_row;
  lines_to_read = lines_after_iMCU_row - lines_to_skip;
  if (cinfo->inputctl->has_multiple_scans) {
    if (cinfo->upsample->need_context_rows) {
      cinfo->output_scanline += lines_to_skip;
      cinfo->output_iMCU_row += lines_to_skip / lines_per_iMCU_row;
      main_ptr->iMCU_row_ctr += lines_to_skip / lines_per_iMCU_row;
      read_and_discard_scanlines(cinfo, lines_to_read);
    } else {
      cinfo->output_scanline += lines_to_skip;
      cinfo->output_iMCU_row += lines_to_skip / lines_per_iMCU_row;
      increment_simple_rowgroup_ctr(cinfo, lines_to_read);
    }
    upsample->rows_to_go = cinfo->output_height - cinfo->output_scanline;
    return num_lines;
  }
  for (i = 0; i < lines_to_skip; i += lines_per_iMCU_row) {
    for (y = 0; y < coef->MCU_rows_per_iMCU_row; y++) {
      for (x = 0; x < cinfo->MCUs_per_row; x++) {
        (*cinfo->entropy->decode_mcu) (cinfo, NULL);
      }
    }
    cinfo->input_iMCU_row++;
    cinfo->output_iMCU_row++;
    if (cinfo->input_iMCU_row < cinfo->total_iMCU_rows)
      start_iMCU_row(cinfo);
    else
      (*cinfo->inputctl->finish_input_pass) (cinfo);
  }
  cinfo->output_scanline += lines_to_skip;
  if (cinfo->upsample->need_context_rows) {
    main_ptr->iMCU_row_ctr += lines_to_skip / lines_per_iMCU_row;
    read_and_discard_scanlines(cinfo, lines_to_read);
  } else {
    increment_simple_rowgroup_ctr(cinfo, lines_to_read);
  }
  upsample->rows_to_go = cinfo->output_height - cinfo->output_scanline;
  return num_lines;
}