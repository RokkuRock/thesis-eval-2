void scramble(FILE* keyFile){
	for (int j = 0; j < 16; ++j)
	{
		char temp = 0;
		for (int i = 0; i < 256; ++i)
		{
			scrambleAsciiTables[j][i] = i;
		}
		if (keyFile != NULL){
			int size;
			char extractedString[BUFFER_SIZE] = "";
			while((size = fread(extractedString, 1, BUFFER_SIZE, keyFile)) > 0){
				for (int i = 0; i < size; ++i)
				{
					temp = scrambleAsciiTables[j][i%256];
					scrambleAsciiTables[j][i%256] = scrambleAsciiTables[j][(unsigned char)(extractedString[i])];
					scrambleAsciiTables[j][(unsigned char)(extractedString[i])] = temp;
				}
			}
			rewind(keyFile);
		} else {
			unsigned char random256;
			for (int i = 0; i < 10 * 256; ++i)
			{
				random256 = generateNumber() ^ passPhrase[passIndex];
				passIndex++;
				passIndex %= 16384;
				temp = scrambleAsciiTables[j][i%256];
				scrambleAsciiTables[j][i%256] = scrambleAsciiTables[j][random256];
				scrambleAsciiTables[j][random256] = temp;
			}
		}
	}
}