		normChunk = function( s , l ) {
			return ( ! s.match( ore ) || l === 1 ) && parseFloat( s ) || s.replace( snre , ' ' ).replace( sre , '' ) || 0 ;	 
		} ,