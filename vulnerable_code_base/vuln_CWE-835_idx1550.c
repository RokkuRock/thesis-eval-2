ssize_t mgs_transport_read(gnutls_transport_ptr_t ptr,
                           void *buffer, size_t len)
{
    mgs_handle_t *ctxt = ptr;
    apr_size_t in = len;
    apr_read_type_e block = ctxt->input_block;
    ctxt->input_rc = APR_SUCCESS;
    if (!len || buffer == NULL)
    {
        return 0;
    }
    if (!ctxt->input_bb)
    {
        ctxt->input_rc = APR_EOF;
        gnutls_transport_set_errno(ctxt->session, ECONNABORTED);
        return -1;
    }
    if (APR_BRIGADE_EMPTY(ctxt->input_bb))
    {
        apr_status_t rc = ap_get_brigade(ctxt->input_filter->next,
                                         ctxt->input_bb, AP_MODE_READBYTES,
                                         ctxt->input_block, in);
        if (APR_STATUS_IS_EAGAIN(rc) || APR_STATUS_IS_EINTR(rc)
            || (rc == APR_SUCCESS
                && APR_BRIGADE_EMPTY(ctxt->input_bb)))
        {
            ctxt->input_rc = (rc != APR_SUCCESS ? rc : APR_EINTR);
            gnutls_transport_set_errno(ctxt->session,
                                       EAI_APR_TO_RAW(ctxt->input_rc));
            return -1;
        }
        if (ctxt->input_block == APR_BLOCK_READ
            && APR_STATUS_IS_TIMEUP(rc)
            && APR_BRIGADE_EMPTY(ctxt->input_bb))
        {
            ctxt->input_rc = rc;
            gnutls_transport_set_errno(ctxt->session, EAGAIN);
            return -1;
        }
        if (rc != APR_SUCCESS)
        {
            ap_log_cerror(APLOG_MARK, APLOG_INFO, rc, ctxt->c,
                          "%s: Unexpected error!", __func__);
            apr_brigade_cleanup(ctxt->input_bb);
            ctxt->input_bb = NULL;
            gnutls_transport_set_errno(ctxt->session, EIO);
            return -1;
        }
    }
    ctxt->input_rc = brigade_consume(ctxt->input_bb, block, buffer, &len);
    if (ctxt->input_rc == APR_SUCCESS)
    {
        return (ssize_t) len;
    }
    if (APR_STATUS_IS_EAGAIN(ctxt->input_rc)
        || APR_STATUS_IS_EINTR(ctxt->input_rc))
    {
        if (len == 0)
        {
            gnutls_transport_set_errno(ctxt->session,
                                       EAI_APR_TO_RAW(ctxt->input_rc));
            return -1;
        }
        return (ssize_t) len;
    }
    apr_brigade_cleanup(ctxt->input_bb);
    ctxt->input_bb = NULL;
    if (APR_STATUS_IS_EOF(ctxt->input_rc) && len)
    {
        return (ssize_t) len;
    }
    gnutls_transport_set_errno(ctxt->session, EIO);
    return -1;
}