static void add_password(AUTH_HDR *request, unsigned char type, CONST char *password, char *secret)
{
	MD5_CTX md5_secret, my_md5;
	unsigned char misc[AUTH_VECTOR_LEN];
	int i;
	int length = strlen(password);
	unsigned char hashed[256 + AUTH_PASS_LEN];	 
	unsigned char *vector;
	attribute_t *attr;
	if (length > MAXPASS) {				 
		length = MAXPASS;
	}
	if (length == 0) {
		length = AUTH_PASS_LEN;			 
	} if ((length & (AUTH_PASS_LEN - 1)) != 0) {
		length += (AUTH_PASS_LEN - 1);		 
		length &= ~(AUTH_PASS_LEN - 1);		 
	}						 
	memset(hashed, 0, length);
	memcpy(hashed, password, strlen(password));
	attr = find_attribute(request, PW_PASSWORD);
	if (type == PW_PASSWORD) {
		vector = request->vector;
	} else {
		vector = attr->data;			 
	}
	MD5Init(&md5_secret);
	MD5Update(&md5_secret, (unsigned char *) secret, strlen(secret));
	my_md5 = md5_secret;				 
	MD5Update(&my_md5, vector, AUTH_VECTOR_LEN);
	MD5Final(misc, &my_md5);			 
	xor(hashed, misc, AUTH_PASS_LEN);
	for (i = 1; i < (length >> 4); i++) {
		my_md5 = md5_secret;			 
		MD5Update(&my_md5, &hashed[(i-1) * AUTH_PASS_LEN], AUTH_PASS_LEN);
		MD5Final(misc, &my_md5);			 
		xor(&hashed[i * AUTH_PASS_LEN], misc, AUTH_PASS_LEN);
	}
	if (type == PW_OLD_PASSWORD) {
		attr = find_attribute(request, PW_OLD_PASSWORD);
	}
	if (!attr) {
		add_attribute(request, type, hashed, length);
	} else {
		memcpy(attr->data, hashed, length);  
	}
}