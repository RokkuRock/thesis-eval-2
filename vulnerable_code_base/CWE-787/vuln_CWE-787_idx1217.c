void gps_tracker( void )
{
	ssize_t unused;
    int gpsd_sock;
    char line[256], *temp;
    struct sockaddr_in gpsd_addr;
    int ret, is_json, pos;
    fd_set read_fd;
    struct timeval timeout;
    pos = 0;
    gpsd_sock = socket( AF_INET, SOCK_STREAM, 0 );
    if( gpsd_sock < 0 ) {
        return;
    }
    gpsd_addr.sin_family      = AF_INET;
    gpsd_addr.sin_port        = htons( 2947 );
    gpsd_addr.sin_addr.s_addr = inet_addr( "127.0.0.1" );
    if( connect( gpsd_sock, (struct sockaddr *) &gpsd_addr,
                 sizeof( gpsd_addr ) ) < 0 ) {
        return;
    }
    FD_ZERO(&read_fd);
    FD_SET(gpsd_sock, &read_fd);
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    is_json = select(gpsd_sock + 1, &read_fd, NULL, NULL, &timeout);
    if (is_json) {
    	if( recv( gpsd_sock, line, sizeof( line ) - 1, 0 ) <= 0 )
    		return;
    	is_json = (line[0] == '{');
    	if (is_json) {
			memset( line, 0, sizeof( line ) );
			strcpy(line, "?WATCH={\"json\":true};\n");
			if( send( gpsd_sock, line, 22, 0 ) != 22 )
				return;
			memset(line, 0, sizeof(line));
			if( recv( gpsd_sock, line, sizeof( line ) - 1, 0 ) <= 0 )
				return;
			if (strncmp(line, "{\"class\":\"DEVICES\",\"devices\":[]}", 32) == 0) {
				close(gpsd_sock);
				return;
			} else {
				pos = strlen(line);
			}
    	}
    }
    while( G.do_exit == 0 )
    {
        usleep( 500000 );
        memset( G.gps_loc, 0, sizeof( float ) * 5 );
        if (is_json) {
        	if (pos == sizeof( line )) {
        		memset(line, 0, sizeof(line));
        		pos = 0;
        	}
        	if( recv( gpsd_sock, line + pos, sizeof( line ) - 1, 0 ) <= 0 )
        		return;
        	temp = strstr(line, "{\"class\":\"TPV\"");
        	if (temp == NULL) {
        		continue;
        	}
        	if (strchr(temp, '}') == NULL) {
        		pos = strlen(temp);
        		if (temp != line) {
        			memmove(line, temp, pos);
        			memset(line + pos, 0, sizeof(line) - pos);
        		}
        	}
        	temp = strstr(temp, "\"lat\":");
			if (temp == NULL) {
				continue;
			}
			ret = sscanf(temp + 6, "%f", &G.gps_loc[0]);
			temp = strstr(temp, "\"lon\":");
			if (temp == NULL) {
				continue;
			}
			ret = sscanf(temp + 6, "%f", &G.gps_loc[1]);
			temp = strstr(temp, "\"alt\":");
			if (temp == NULL) {
				continue;
			}
			ret = sscanf(temp + 6, "%f", &G.gps_loc[4]);
			temp = strstr(temp, "\"speed\":");
			if (temp == NULL) {
				continue;
			}
			ret = sscanf(temp + 6, "%f", &G.gps_loc[2]);
			temp = strstr(temp, "{\"class\":\"TPV\"");
			if (temp == NULL) {
				memset( line, 0, sizeof( line ) );
				pos = 0;
			} else {
				pos = strlen(temp);
				memmove(line, temp, pos);
				memset(line + pos, 0, sizeof(line) - pos);
			}
        } else {
        	memset( line, 0, sizeof( line ) );
			snprintf( line,  sizeof( line ) - 1, "PVTAD\r\n" );
			if( send( gpsd_sock, line, 7, 0 ) != 7 )
				return;
			memset( line, 0, sizeof( line ) );
			if( recv( gpsd_sock, line, sizeof( line ) - 1, 0 ) <= 0 )
				return;
			if( memcmp( line, "GPSD,P=", 7 ) != 0 )
				continue;
			if( line[7] == '?' )
				continue;
			ret = sscanf( line + 7, "%f %f", &G.gps_loc[0], &G.gps_loc[1] );
			if( ( temp = strstr( line, "V=" ) ) == NULL ) continue;
			ret = sscanf( temp + 2, "%f", &G.gps_loc[2] );  
			if( ( temp = strstr( line, "T=" ) ) == NULL ) continue;
			ret = sscanf( temp + 2, "%f", &G.gps_loc[3] );  
			if( ( temp = strstr( line, "A=" ) ) == NULL ) continue;
			ret = sscanf( temp + 2, "%f", &G.gps_loc[4] );  
        }
        if (G.record_data)
			fputs( line, G.f_gps );
		G.save_gps = 1;
        if (G.do_exit == 0)
		{
			unused = write( G.gc_pipe[1], G.gps_loc, sizeof( float ) * 5 );
			kill( getppid(), SIGUSR2 );
		}
    }
}