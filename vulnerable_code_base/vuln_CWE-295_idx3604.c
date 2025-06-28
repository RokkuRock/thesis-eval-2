int main(int argc, char **argv)
{
  MYSQL mysql;
  option_string *eptr;
  MY_INIT(argv[0]);
  my_getopt_use_args_separator= TRUE;
  if (load_defaults("my",load_default_groups,&argc,&argv))
  {
    my_end(0);
    exit(1);
  }
  my_getopt_use_args_separator= FALSE;
  defaults_argv=argv;
  if (get_options(&argc,&argv))
  {
    free_defaults(defaults_argv);
    my_end(0);
    exit(1);
  }
  if (auto_generate_sql)
    srandom((uint)time(NULL));
  delimiter_length= strlen(delimiter);
  if (argc > 2)
  {
    fprintf(stderr,"%s: Too many arguments\n",my_progname);
    free_defaults(defaults_argv);
    my_end(0);
    exit(1);
  }
  mysql_init(&mysql);
  if (opt_compress)
    mysql_options(&mysql,MYSQL_OPT_COMPRESS,NullS);
#ifdef HAVE_OPENSSL
  if (opt_use_ssl)
  {
    mysql_ssl_set(&mysql, opt_ssl_key, opt_ssl_cert, opt_ssl_ca,
                  opt_ssl_capath, opt_ssl_cipher);
    mysql_options(&mysql, MYSQL_OPT_SSL_CRL, opt_ssl_crl);
    mysql_options(&mysql, MYSQL_OPT_SSL_CRLPATH, opt_ssl_crlpath);
  }
#endif
  if (opt_protocol)
    mysql_options(&mysql,MYSQL_OPT_PROTOCOL,(char*)&opt_protocol);
#if defined (_WIN32) && !defined (EMBEDDED_LIBRARY)
  if (shared_memory_base_name)
    mysql_options(&mysql,MYSQL_SHARED_MEMORY_BASE_NAME,shared_memory_base_name);
#endif
  mysql_options(&mysql, MYSQL_SET_CHARSET_NAME, default_charset);
  if (opt_plugin_dir && *opt_plugin_dir)
    mysql_options(&mysql, MYSQL_PLUGIN_DIR, opt_plugin_dir);
  if (opt_default_auth && *opt_default_auth)
    mysql_options(&mysql, MYSQL_DEFAULT_AUTH, opt_default_auth);
  mysql_options(&mysql, MYSQL_OPT_CONNECT_ATTR_RESET, 0);
  mysql_options4(&mysql, MYSQL_OPT_CONNECT_ATTR_ADD,
                 "program_name", "mysqlslap");
  if (using_opt_enable_cleartext_plugin)
    mysql_options(&mysql, MYSQL_ENABLE_CLEARTEXT_PLUGIN, 
                  (char*) &opt_enable_cleartext_plugin);
  if (!opt_only_print) 
  {
    if (!(mysql_real_connect(&mysql, host, user, opt_password,
                             NULL, opt_mysql_port,
                             opt_mysql_unix_port, connect_flags)))
    {
      fprintf(stderr,"%s: Error when connecting to server: %s\n",
              my_progname,mysql_error(&mysql));
      free_defaults(defaults_argv);
      my_end(0);
      exit(1);
    }
  }
  pthread_mutex_init(&counter_mutex, NULL);
  pthread_cond_init(&count_threshhold, NULL);
  pthread_mutex_init(&sleeper_mutex, NULL);
  pthread_cond_init(&sleep_threshhold, NULL);
  eptr= engine_options;
  do
  {
    uint *current;
    if (verbose >= 2)
      printf("Starting Concurrency Test\n");
    if (*concurrency)
    {
      for (current= concurrency; current && *current; current++)
        concurrency_loop(&mysql, *current, eptr);
    }
    else
    {
      uint infinite= 1;
      do {
        concurrency_loop(&mysql, infinite, eptr);
      }
      while (infinite++);
    }
    if (!opt_preserve)
      drop_schema(&mysql, create_schema_string);
  } while (eptr ? (eptr= eptr->next) : 0);
  pthread_mutex_destroy(&counter_mutex);
  pthread_cond_destroy(&count_threshhold);
  pthread_mutex_destroy(&sleeper_mutex);
  pthread_cond_destroy(&sleep_threshhold);
  if (!opt_only_print) 
    mysql_close(&mysql);  
  my_free(opt_password);
  my_free(concurrency);
  statement_cleanup(create_statements);
  statement_cleanup(query_statements);
  statement_cleanup(pre_statements);
  statement_cleanup(post_statements);
  option_cleanup(engine_options);
#if defined (_WIN32) && !defined (EMBEDDED_LIBRARY)
  my_free(shared_memory_base_name);
#endif
  free_defaults(defaults_argv);
  my_end(my_end_arg);
  return 0;
}