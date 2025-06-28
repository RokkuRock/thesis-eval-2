static int getSingletonPos(const char* str)
{
	int result =-1;
	int i=0;
	int len = 0;
	if( str && ((len=strlen(str))>0) ){
		for( i=0; i<len ; i++){
			if( isIDSeparator(*(str+i)) ){
				if( i==1){
					result =0;
					break;
				} else {
					if( isIDSeparator(*(str+i+2)) ){
						result = i+1;
						break;
					}
				}
			}
		} 
	}
	return result;
}