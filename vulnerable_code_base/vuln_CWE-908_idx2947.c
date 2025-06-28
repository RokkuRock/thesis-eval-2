vips_tracked_malloc( size_t size )
{
        void *buf;
	vips_tracked_init(); 
	size += 16;
        if( !(buf = g_try_malloc( size )) ) {
#ifdef DEBUG
		g_assert_not_reached();
#endif  
		vips_error( "vips_tracked", 
			_( "out of memory --- size == %dMB" ), 
			(int) (size / (1024.0 * 1024.0))  );
		g_warning( _( "out of memory --- size == %dMB" ), 
			(int) (size / (1024.0 * 1024.0))  );
                return( NULL );
	}
	g_mutex_lock( vips_tracked_mutex );
	*((size_t *)buf) = size;
	buf = (void *) ((char *)buf + 16);
	vips_tracked_mem += size;
	if( vips_tracked_mem > vips_tracked_mem_highwater ) 
		vips_tracked_mem_highwater = vips_tracked_mem;
	vips_tracked_allocs += 1;
#ifdef DEBUG_VERBOSE
	printf( "vips_tracked_malloc: %p, %zd bytes\n", buf, size ); 
#endif  
	g_mutex_unlock( vips_tracked_mutex );
	VIPS_GATE_MALLOC( size ); 
        return( buf );
}