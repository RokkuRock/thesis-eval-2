static pj_status_t STATUS_FROM_SSL_ERR(char *action, pj_ssl_sock_t *ssock,
				       unsigned long err)
{
    int level = 0;
    int len = 0;  
    ERROR_LOG("STATUS_FROM_SSL_ERR", err, ssock);
    level++;
    if (err == SSL_ERROR_SSL) {
	err = ERR_get_error();
	ERROR_LOG("STATUS_FROM_SSL_ERR", err, ssock);
    }
    ssock->last_err = err;
    return GET_STATUS_FROM_SSL_ERR(err);
}