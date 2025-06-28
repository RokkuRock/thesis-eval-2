int main(int argc, char **argv)
{
  test_cmp_parameters inParam;
  FILE *fbase=NULL, *ftest=NULL;
  int same = 0;
  char lbase[256];
  char strbase[256];
  char ltest[256];
  char strtest[256];
  if( parse_cmdline_cmp(argc, argv, &inParam) == 1 )
    {
    compare_dump_files_help_display();
    goto cleanup;
    }
  printf("******Parameters********* \n");
  printf(" base_filename = %s\n"
    " test_filename = %s\n",
    inParam.base_filename, inParam.test_filename);
  printf("************************* \n");
  printf("Try to open: %s for reading ... ", inParam.base_filename);
  if((fbase = fopen(inParam.base_filename, "rb"))==NULL)
    {
    goto cleanup;
    }
  printf("Ok.\n");
  printf("Try to open: %s for reading ... ", inParam.test_filename);
  if((ftest = fopen(inParam.test_filename, "rb"))==NULL)
    {
    goto cleanup;
    }
  printf("Ok.\n");
  while (fgets(lbase, sizeof(lbase), fbase) && fgets(ltest,sizeof(ltest),ftest))
    {
    int nbase = sscanf(lbase, "%255[^\r\n]", strbase);
    int ntest = sscanf(ltest, "%255[^\r\n]", strtest);
    assert( nbase != 255 && ntest != 255 );
    if( nbase != 1 || ntest != 1 )
      {
      fprintf(stderr, "could not parse line from files\n" );
      goto cleanup;
      }
    if( strcmp( strbase, strtest ) != 0 )
      {
      fprintf(stderr,"<%s> vs. <%s>\n", strbase, strtest);
      goto cleanup;
      }
    }
  same = 1;
  printf("\n***** TEST SUCCEED: Files are the same. *****\n");
cleanup:
  if(fbase) fclose(fbase);
  if(ftest) fclose(ftest);
  free(inParam.base_filename);
  free(inParam.test_filename);
  return same ? EXIT_SUCCESS : EXIT_FAILURE;
}