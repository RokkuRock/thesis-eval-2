module.exports = function( a , b ) {
	var re = /(^([+-]?(?:\d*)(?:\.\d*)?(?:[eE][+-]?\d+)?)?$|^0x[\da-fA-F]+$|\d+)/g ,
		sre = /^\s+|\s+$/g ,    
		snre = /\s+/g ,         
		dre = /(^([\w ]+,?[\w ]+)?[\w ]+,?[\w ]+\d+:\d+(:\d+)?[\w ]?|^\d{1,4}[/-]\d{1,4}[/-]\d{1,4}|^\w+, \w+ \d+, \d{4})/ ,
		hre = /^0x[0-9a-f]+$/i ,
		ore = /^0/ ,
		i = function( s ) {
			return ( '' + s ).toLowerCase().replace( sre , '' ) ;
		} ,
		x = i( a ) || '' ,
		y = i( b ) || '' ,
		xN = x.replace( re , '\0$1\0' ).replace( /\0$/ , '' )
			.replace( /^\0/ , '' )
			.split( '\0' ) ,
		yN = y.replace( re , '\0$1\0' ).replace( /\0$/ , '' )
			.replace( /^\0/ , '' )
			.split( '\0' ) ,
		xD = parseInt( x.match( hre ) , 16 ) || ( xN.length !== 1 && Date.parse( x ) ) ,
		yD = parseInt( y.match( hre ) , 16 ) || xD && y.match( dre ) && Date.parse( y ) || null ,
		normChunk = function( s , l ) {
			return ( ! s.match( ore ) || l === 1 ) && parseFloat( s ) || s.replace( snre , ' ' ).replace( sre , '' ) || 0 ;	 
		} ,
		oFxNcL , oFyNcL ;
	if ( yD ) {
		if ( xD < yD ) { return -1 ; }
		else if ( xD > yD ) { return 1 ; }
	}
	for( var cLoc = 0 , xNl = xN.length , yNl = yN.length , numS = Math.max( xNl , yNl ) ; cLoc < numS ; cLoc ++ ) {
		oFxNcL = normChunk( xN[cLoc] , xNl ) ;
		oFyNcL = normChunk( yN[cLoc] , yNl ) ;
		if ( isNaN( oFxNcL ) !== isNaN( oFyNcL ) ) { return ( isNaN( oFxNcL ) ) ? 1 : -1 ; }
		else if ( typeof oFxNcL !== typeof oFyNcL ) {
			oFxNcL += '' ;
			oFyNcL += '' ;
		}
		if ( oFxNcL < oFyNcL ) { return -1 ; }
		if ( oFxNcL > oFyNcL ) { return 1 ; }
	}
	return 0 ;
} ;