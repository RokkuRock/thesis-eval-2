int main(int argc, char *argv[])
{
   libettercap_init();
   ef_globals_alloc();
   select_text_interface();
   libettercap_ui_init();
   fprintf(stdout, "\n" EC_COLOR_BOLD "%s %s" EC_COLOR_END " copyright %s %s\n\n", 
                      PROGRAM, EC_VERSION, EC_COPYRIGHT, EC_AUTHORS);
   EF_GBL->lineno = 1;
   parse_options(argc, argv);
   if (EF_GBL_OPTIONS->source_file) {
      yyin = fopen(EF_GBL_OPTIONS->source_file, "r");
      if (yyin == NULL)
         FATAL_ERROR("Input file not found !");
   } else {
      FATAL_ERROR("No source file.");
   }
   setbuf(yyin, NULL);
   setbuf(stdout, NULL);
   setbuf(stderr, NULL);
   load_tables();
   load_constants();
   fprintf(stdout, "\n Parsing source file \'%s\' ", EF_GBL_OPTIONS->source_file);
   fflush(stdout);
   ef_debug(1, "\n");
   if (yyparse() == 0)
      fprintf(stdout, " done.\n\n");
   else
      fprintf(stdout, "\n\nThe script contains errors...\n\n");
   if (write_output() != E_SUCCESS)
      FATAL_ERROR("Cannot write output file (%s)", EF_GBL_OPTIONS->output_file);
   ef_globals_free();
   return 0;
}