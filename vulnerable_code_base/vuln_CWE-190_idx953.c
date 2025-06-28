JSON_read(int fd)
{
    uint32_t hsize, nsize;
    char *str;
    cJSON *json = NULL;
    int rc;
    if (Nread(fd, (char*) &nsize, sizeof(nsize), Ptcp) >= 0) {
	hsize = ntohl(nsize);
	str = (char *) calloc(sizeof(char), hsize+1);	 
	if (str != NULL) {
	    rc = Nread(fd, str, hsize, Ptcp);
	    if (rc >= 0) {
		if (rc == hsize) {
		    json = cJSON_Parse(str);
		}
		else {
		    printf("WARNING:  Size of data read does not correspond to offered length\n");
		}
	    }
	}
	free(str);
    }
    return json;
}