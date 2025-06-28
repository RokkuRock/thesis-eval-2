start_pass_merged_upsample(j_decompress_ptr cinfo)
{
  my_upsample_ptr upsample = (my_upsample_ptr)cinfo->upsample;
  upsample->spare_full = FALSE;
  upsample->rows_to_go = cinfo->output_height;
}