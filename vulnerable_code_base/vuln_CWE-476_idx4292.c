merged_1v_upsample(j_decompress_ptr cinfo, JSAMPIMAGE input_buf,
                   JDIMENSION *in_row_group_ctr,
                   JDIMENSION in_row_groups_avail, JSAMPARRAY output_buf,
                   JDIMENSION *out_row_ctr, JDIMENSION out_rows_avail)
{
  my_upsample_ptr upsample = (my_upsample_ptr)cinfo->upsample;
  (*upsample->upmethod) (cinfo, input_buf, *in_row_group_ctr,
                         output_buf + *out_row_ctr);
  (*out_row_ctr)++;
  (*in_row_group_ctr)++;
}