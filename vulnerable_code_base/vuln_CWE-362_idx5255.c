vips_foreign_load_start( VipsImage *out, void *a, void *b )
{
	VipsForeignLoad *load = VIPS_FOREIGN_LOAD( b );
	VipsForeignLoadClass *class = VIPS_FOREIGN_LOAD_GET_CLASS( load );
	if( !load->real ) {
		if( !(load->real = vips_foreign_load_temp( load )) )
			return( NULL );
#ifdef DEBUG
		printf( "vips_foreign_load_start: triggering ->load()\n" );
#endif  
		load->real->progress_signal = load->out;
		g_object_set_qdata( G_OBJECT( load->real ), 
			vips__foreign_load_operation, load ); 
		if( class->load( load ) ||
			vips_image_pio_input( load->real ) ) 
			return( NULL );
		if( !vips_foreign_load_iscompat( load->real, out ) )
			return( NULL );
		vips_image_pipelinev( load->out, load->out->dhint, 
			load->real, NULL );
	}
	return( vips_region_new( load->real ) );
}