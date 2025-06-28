static int getStrrtokenPos(char* str, int savedPos)
{
	int result =-1;
	int i;
	for(i=savedPos-1; i>=0; i--) {
		if(isIDSeparator(*(str+i)) ){
			if(i>=2 && isIDSeparator(*(str+i-2)) ){
				result = i-2;
			} else {
				result = i;
			}
			break;
		}
	}
	if(result < 1){
		result =-1;
	}
	return result;
}