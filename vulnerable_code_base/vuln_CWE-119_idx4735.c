int squidclamav_check_preview_handler(char *preview_data, int preview_data_len, ci_request_t * req)
{
     ci_headers_list_t *req_header;
     struct http_info httpinf;
     av_req_data_t *data = ci_service_data(req); 
     char *clientip;
     struct hostent *clientname;
     unsigned long ip;
     char *username;
     char *content_type;
     ci_off_t content_length;
     char *chain_ret = NULL;
     char *ret = NULL;
     int chkipdone = 0;
     ci_debug_printf(1, "DEBUG squidclamav_check_preview_handler: processing preview header.\n");
     if (preview_data_len)
	ci_debug_printf(1, "DEBUG squidclamav_check_preview_handler: preview data size is %d\n", preview_data_len);
     if ((req_header = ci_http_request_headers(req)) == NULL) {
	ci_debug_printf(0, "ERROR squidclamav_check_preview_handler: bad http header, aborting.\n");
	return CI_ERROR;
     }
     if ((username = ci_headers_value(req->request_header, "X-Authenticated-User")) != NULL) {
	ci_debug_printf(2, "DEBUG squidclamav_check_preview_handler: X-Authenticated-User: %s\n", username);
        if (simple_pattern_compare(username, TRUSTUSER) == 1) {
           ci_debug_printf(1, "DEBUG squidclamav_check_preview_handler: No squidguard and antivir check (TRUSTUSER match) for user: %s\n", username);
	   return CI_MOD_ALLOW204;
        }
     } else {
	username = (char *)malloc(sizeof(char)*2);
	strcpy(username, "-");
     }
     if ((clientip = ci_headers_value(req->request_header, "X-Client-IP")) != NULL) {
	ci_debug_printf(2, "DEBUG squidclamav_check_preview_handler: X-Client-IP: %s\n", clientip);
	ip = inet_addr(clientip);
	chkipdone = 0;
	if (dnslookup == 1) {
		if ( (clientname = gethostbyaddr((char *)&ip, sizeof(ip), AF_INET)) != NULL) {
			if (clientname->h_name != NULL) {
				if (client_pattern_compare(clientip, clientname->h_name) > 0) {
				   ci_debug_printf(1, "DEBUG squidclamav_check_preview_handler: No squidguard and antivir check (TRUSTCLIENT match) for client: %s(%s)\n", clientname->h_name, clientip);
				   return CI_MOD_ALLOW204;
				}
				chkipdone = 1;
			}
		  }
	}
	if (chkipdone == 0) {
		if (client_pattern_compare(clientip, NULL) > 0) {
		   ci_debug_printf(1, "DEBUG squidclamav_check_preview_handler: No squidguard and antivir check (TRUSTCLIENT match) for client: %s\n", clientip);
		   return CI_MOD_ALLOW204;
		}
	}
     } else {
	clientip = (char *)malloc(sizeof(char)*2);
	strcpy(clientip, "-");
     }
     if (!extract_http_info(req, req_header, &httpinf)) {
	ci_debug_printf(1, "DEBUG squidclamav_check_preview_handler: bad http header, aborting.\n");
	return CI_MOD_ALLOW204;
     }
     ci_debug_printf(2, "DEBUG squidclamav_check_preview_handler: URL requested: %s\n", httpinf.url);
     if (simple_pattern_compare(httpinf.url, WHITELIST) == 1) {
           ci_debug_printf(1, "DEBUG squidclamav_check_preview_handler: No squidguard and antivir check (WHITELIST match) for url: %s\n", httpinf.url);
	   return CI_MOD_ALLOW204;
     }
     if (usepipe == 1) {
	ci_debug_printf(2, "DEBUG squidclamav_check_preview_handler: Sending request to chained program: %s\n", squidguard);
	ci_debug_printf(2, "DEBUG squidclamav_check_preview_handler: Request: %s %s %s %s\n", httpinf.url,clientip,username,httpinf.method);
	fprintf(sgfpw,"%s %s %s %s\n",httpinf.url,clientip,username,httpinf.method);
	fflush(sgfpw);
	chain_ret = (char *)malloc(sizeof(char)*MAX_URL_SIZE);
	if (chain_ret != NULL) {
	   ret = fgets(chain_ret,MAX_URL_SIZE,sgfpr);
	   if ((ret != NULL) && (strlen(chain_ret) > 1)) {
		ci_debug_printf(1, "DEBUG squidclamav_check_preview_handler: Chained program redirection received: %s\n", chain_ret);
		if (logredir)
		   ci_debug_printf(0, "INFO Chained program redirection received: %s\n", chain_ret);
		data->blocked = 1;
		generate_redirect_page(strtok(chain_ret, " "), req, data);
	        xfree(chain_ret);
	        chain_ret = NULL;
	        return CI_MOD_CONTINUE;
	   }
	   xfree(chain_ret);
	   chain_ret = NULL;
	}
     }
     if (strcmp(httpinf.method, "CONNECT") == 0) {
	ci_debug_printf(2, "DEBUG squidclamav_check_preview_handler: method %s can't be scanned.\n", httpinf.method);
	return CI_MOD_ALLOW204;
     }
     if (simple_pattern_compare(httpinf.url, ABORT) == 1) {
           ci_debug_printf(1, "DEBUG squidclamav_check_preview_handler: No antivir check (ABORT match) for url: %s\n", httpinf.url);
	   return CI_MOD_ALLOW204;
     }
     content_length = ci_http_content_length(req);
     ci_debug_printf(2, "DEBUG squidclamav_check_preview_handler: Content-Length: %d\n", (int)content_length);
     if ((content_length > 0) && (maxsize > 0) && (content_length >= maxsize)) {
	ci_debug_printf(2, "DEBUG squidclamav_check_preview_handler: No antivir check, content-length upper than maxsize (%d > %d)\n", content_length, (int)maxsize);
	return CI_MOD_ALLOW204;
     }
     if ((content_type = http_content_type(req)) != NULL) {
	ci_debug_printf(2, "DEBUG squidclamav_check_preview_handler: Content-Type: %s\n", content_type);
        if (simple_pattern_compare(content_type, ABORTCONTENT)) {
           ci_debug_printf(1, "DEBUG squidclamav_check_preview_handler: No antivir check (ABORTCONTENT match) for content-type: %s\n", content_type);
	   return CI_MOD_ALLOW204;
        }
     }
     if (!data || !ci_req_hasbody(req)) {
	 ci_debug_printf(1, "DEBUG squidclamav_check_preview_handler: No body data, allow 204\n");
          return CI_MOD_ALLOW204;
     }
     if (preview_data_len == 0) {
	ci_debug_printf(1, "DEBUG squidclamav_check_preview_handler: can not begin to scan url: No preview data.\n");
	return CI_MOD_ALLOW204;
     }
     data->url = ci_buffer_alloc(strlen(httpinf.url)+1);
     strcpy(data->url, httpinf.url);
     if (username != NULL) {
	     data->user = ci_buffer_alloc(strlen(username)+1);
	     strcpy(data->user, username);
     } else {
	data->user = NULL;
     }
     if (clientip != NULL) {
	data->clientip = ci_buffer_alloc(strlen(clientip)+1);
	strcpy(data->clientip, clientip);
     } else {
	ci_debug_printf(0, "ERROR squidclamav_check_preview_handler: clientip is null, you must set 'icap_send_client_ip on' into squid.conf\n");
	data->clientip = NULL;
     }
     data->body = ci_simple_file_new(0);
     if ((SEND_PERCENT_BYTES >= 0) && (START_SEND_AFTER == 0)) {
	ci_req_unlock_data(req);
	ci_simple_file_lock_all(data->body);
     }
     if (!data->body)
	return CI_ERROR;
     if (preview_data_len) {
	if (ci_simple_file_write(data->body, preview_data, preview_data_len, ci_req_hasalldata(req)) == CI_ERROR)
		return CI_ERROR;
     }
     return CI_MOD_CONTINUE;
}