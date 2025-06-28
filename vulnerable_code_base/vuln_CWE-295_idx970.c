get_one_option(int optid, const struct my_option *opt,
               char *argument)
{
  my_bool add_option= TRUE;
  switch (optid) {
  case '?':
    printf("%s  Ver %s Distrib %s, for %s (%s)\n",
           my_progname, VER, MYSQL_SERVER_VERSION, SYSTEM_TYPE, MACHINE_TYPE);
    puts(ORACLE_WELCOME_COPYRIGHT_NOTICE("2000"));
    puts("MySQL utility for upgrading databases to new MySQL versions.\n");
    my_print_help(my_long_options);
    exit(0);
    break;
  case '#':
    DBUG_PUSH(argument ? argument : default_dbug_option);
    add_option= FALSE;
    debug_check_flag= 1;
    break;
  case 'p':
    if (argument == disabled_my_option)
      argument= (char*) "";			 
    tty_password= 1;
    add_option= FALSE;
    if (argument)
    {
      add_one_option(&ds_args, opt, argument);
      while (*argument)
        *argument++= 'x';                        
      tty_password= 0;
    }
    break;
  case 't':
    my_stpnmov(opt_tmpdir, argument, sizeof(opt_tmpdir));
    add_option= FALSE;
    break;
  case 'k':                                      
  case 'v':  
  case 'f':  
  case 's':                                      
  case OPT_WRITE_BINLOG:                         
    add_option= FALSE;
    break;
  case 'h':  
  case 'W':  
  case 'P':  
  case 'S':  
  case OPT_MYSQL_PROTOCOL:  
  case OPT_SHARED_MEMORY_BASE_NAME:  
  case OPT_PLUGIN_DIR:                           
  case OPT_DEFAULT_AUTH:                         
    add_one_option(&conn_args, opt, argument);
    break;
  }
  if (add_option)
  {
    add_one_option(&ds_args, opt, argument);
  }
  return 0;
}