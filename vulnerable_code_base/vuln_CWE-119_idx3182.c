de_dotdot( char* file )
    {
    char* cp;
    char* cp2;
    int l;
    while ( ( cp = strstr( file, "//") ) != (char*) 0 )
	{
	for ( cp2 = cp + 2; *cp2 == '/'; ++cp2 )
	    continue;
	(void) strcpy( cp + 1, cp2 );
	}
    while ( strncmp( file, "./", 2 ) == 0 )
	(void) memmove( file, file + 2, strlen( file ) - 1 );
    while ( ( cp = strstr( file, "/./") ) != (char*) 0 )
	(void) memmove( cp, cp + 2, strlen( file ) - 1 );
    for (;;)
	{
	while ( strncmp( file, "../", 3 ) == 0 )
	    (void) memmove( file, file + 3, strlen( file ) - 2 );
	cp = strstr( file, "/../" );
	if ( cp == (char*) 0 )
	    break;
	for ( cp2 = cp - 1; cp2 >= file && *cp2 != '/'; --cp2 )
	    continue;
	(void) strcpy( cp2 + 1, cp + 4 );
	}
    while ( ( l = strlen( file ) ) > 3 &&
	    strcmp( ( cp = file + l - 3 ), "/.." ) == 0 )
	{
	for ( cp2 = cp - 1; cp2 >= file && *cp2 != '/'; --cp2 )
	    continue;
	if ( cp2 < file )
	    break;
	*cp2 = '\0';
	}
    }