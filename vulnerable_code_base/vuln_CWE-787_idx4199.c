void libxsmm_sparse_csc_reader( libxsmm_generated_code* io_generated_code,
                                const char*             i_csc_file_in,
                                unsigned int**          o_row_idx,
                                unsigned int**          o_column_idx,
                                double**                o_values,
                                unsigned int*           o_row_count,
                                unsigned int*           o_column_count,
                                unsigned int*           o_element_count ) {
  FILE *l_csc_file_handle;
  const unsigned int l_line_length = 512;
  char l_line[512 +1];
  unsigned int l_header_read = 0;
  unsigned int* l_column_idx_id = NULL;
  unsigned int l_i = 0;
  l_csc_file_handle = fopen( i_csc_file_in, "r" );
  if ( l_csc_file_handle == NULL ) {
    LIBXSMM_HANDLE_ERROR( io_generated_code, LIBXSMM_ERR_CSC_INPUT );
    return;
  }
  while (fgets(l_line, l_line_length, l_csc_file_handle) != NULL) {
    if ( strlen(l_line) == l_line_length ) {
      free(*o_row_idx); free(*o_column_idx); free(*o_values); free(l_column_idx_id);
      *o_row_idx = 0; *o_column_idx = 0; *o_values = 0;
      fclose( l_csc_file_handle );  
      LIBXSMM_HANDLE_ERROR( io_generated_code, LIBXSMM_ERR_CSC_READ_LEN );
      return;
    }
    if ( l_line[0] == '%' ) {
      continue;
    } else {
      if ( l_header_read == 0 ) {
        if ( sscanf(l_line, "%u %u %u", o_row_count, o_column_count, o_element_count) == 3 ) {
          *o_row_idx = (unsigned int*) malloc(sizeof(unsigned int) * (*o_element_count));
          *o_column_idx = (unsigned int*) malloc(sizeof(unsigned int) * ((size_t)(*o_column_count) + 1));
          *o_values = (double*) malloc(sizeof(double) * (*o_element_count));
          l_column_idx_id = (unsigned int*) malloc(sizeof(unsigned int) * (*o_column_count));
          if ( ( *o_row_idx == NULL )      ||
               ( *o_column_idx == NULL )   ||
               ( *o_values == NULL )       ||
               ( l_column_idx_id == NULL ) ) {
            free(*o_row_idx); free(*o_column_idx); free(*o_values); free(l_column_idx_id);
            *o_row_idx = 0; *o_column_idx = 0; *o_values = 0;
            fclose(l_csc_file_handle);  
            LIBXSMM_HANDLE_ERROR( io_generated_code, LIBXSMM_ERR_CSC_ALLOC_DATA );
            return;
          }
          memset(*o_row_idx, 0, sizeof(unsigned int) * (*o_element_count));
          memset(*o_column_idx, 0, sizeof(unsigned int) * ((size_t)(*o_column_count) + 1));
          memset(*o_values, 0, sizeof(double) * (*o_element_count));
          memset(l_column_idx_id, 0, sizeof(unsigned int) * (*o_column_count));
          for (l_i = 0; l_i <= *o_column_count; ++l_i) {
            (*o_column_idx)[l_i] = *o_element_count;
          }
          (*o_column_idx)[0] = 0;
          l_i = 0;
          l_header_read = 1;
        } else {
          LIBXSMM_HANDLE_ERROR( io_generated_code, LIBXSMM_ERR_CSC_READ_DESC );
          fclose( l_csc_file_handle );  
          return;
        }
      } else {
        unsigned int l_row = 0, l_column = 0;
        double l_value = 0;
        if ( sscanf(l_line, "%u %u %lf", &l_row, &l_column, &l_value) != 3 ) {
          free(*o_row_idx); free(*o_column_idx); free(*o_values); free(l_column_idx_id);
          *o_row_idx = 0; *o_column_idx = 0; *o_values = 0;
          fclose(l_csc_file_handle);  
          LIBXSMM_HANDLE_ERROR( io_generated_code, LIBXSMM_ERR_CSC_READ_ELEMS );
          return;
        }
        l_row--;
        l_column--;
        (*o_row_idx)[l_i] = l_row;
        (*o_values)[l_i] = l_value;
        l_i++;
        l_column_idx_id[l_column] = 1;
        (*o_column_idx)[l_column+1] = l_i;
      }
    }
  }
  fclose( l_csc_file_handle );
  if ( l_i != (*o_element_count) ) {
    free(*o_row_idx); free(*o_column_idx); free(*o_values); free(l_column_idx_id);
    *o_row_idx = 0; *o_column_idx = 0; *o_values = 0;
    LIBXSMM_HANDLE_ERROR( io_generated_code, LIBXSMM_ERR_CSC_LEN );
    return;
  }
  if ( l_column_idx_id != NULL ) {
    for ( l_i = 0; l_i < (*o_column_count); l_i++) {
      if ( l_column_idx_id[l_i] == 0 ) {
        (*o_column_idx)[l_i+1] = (*o_column_idx)[l_i];
      }
    }
    free( l_column_idx_id );
  }
}