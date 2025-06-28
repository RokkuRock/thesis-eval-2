merged_2v_upsample(j_decompress_ptr cinfo, JSAMPIMAGE input_buf,
                   JDIMENSION *in_row_group_ctr,
                   JDIMENSION in_row_groups_avail, JSAMPARRAY output_buf,
                   JDIMENSION *out_row_ctr, JDIMENSION out_rows_avail)
{
  my_upsample_ptr upsample = (my_upsample_ptr)cinfo->upsample;
  JSAMPROW work_ptrs[2];
  JDIMENSION num_rows;           
  if (upsample->spare_full) {
    JDIMENSION size = upsample->out_row_width;
    if (cinfo->out_color_space == JCS_RGB565)
      size = cinfo->output_width * 2;
    jcopy_sample_rows(&upsample->spare_row, 0, output_buf + *out_row_ctr, 0, 1,
                      size);
    num_rows = 1;
    upsample->spare_full = FALSE;
  } else {
    num_rows = 2;
    if (num_rows > upsample->rows_to_go)
      num_rows = upsample->rows_to_go;
    out_rows_avail -= *out_row_ctr;
    if (num_rows > out_rows_avail)
      num_rows = out_rows_avail;
    work_ptrs[0] = output_buf[*out_row_ctr];
    if (num_rows > 1) {
      work_ptrs[1] = output_buf[*out_row_ctr + 1];
    } else {
      work_ptrs[1] = upsample->spare_row;
      upsample->spare_full = TRUE;
    }
    (*upsample->upmethod) (cinfo, input_buf, *in_row_group_ctr, work_ptrs);
  }
  *out_row_ctr += num_rows;
  upsample->rows_to_go -= num_rows;
  if (!upsample->spare_full)
    (*in_row_group_ctr)++;
}