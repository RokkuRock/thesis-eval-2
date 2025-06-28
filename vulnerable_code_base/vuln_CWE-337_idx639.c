void decodingXOR(char* extractedString, char* keyString, char* xoredString, int bufferLength)
{
	int i;
	if(isCodingInverted){
		for (i = 0; i < bufferLength; ++i)
		{
			xoredString[i] = unscrambleAsciiTables[keyString[i] & (1+2+4+8)][(unsigned char)extractedString[i]] ^ keyString[i];
		}
	}else{
		for (i = 0; i < bufferLength; ++i)
		{
			xoredString[i] = unscrambleAsciiTables[keyString[i] & (1+2+4+8)][(unsigned char)(extractedString[i] ^ keyString[i])];
		}
	}
}