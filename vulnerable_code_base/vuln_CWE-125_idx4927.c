static char* get_private_subtags(const char* loc_name)
{
	char* 	result =NULL;
	int 	singletonPos = 0;
	int 	len =0; 
	const char* 	mod_loc_name =NULL;
	if( loc_name && (len = strlen(loc_name)>0 ) ){
		mod_loc_name = loc_name ; 
		len   = strlen(mod_loc_name);
		while( (singletonPos = getSingletonPos(mod_loc_name))!= -1){
			if( singletonPos!=-1){ 
				if( (*(mod_loc_name+singletonPos)=='x') || (*(mod_loc_name+singletonPos)=='X') ){		
					if( singletonPos + 2 ==  len){
					}
					else{
						result = estrndup(mod_loc_name + singletonPos+2  , (len -( singletonPos +2) ) );
					}
					break;
				}
				else{
					if( singletonPos + 1 >=  len){
						break;
					} else {
						mod_loc_name = mod_loc_name + singletonPos +1;
						len = strlen(mod_loc_name);
					}
				}
			}
		}  
	}
	return result;
}