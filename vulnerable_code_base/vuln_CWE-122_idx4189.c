PJ_DEF(pj_status_t) pj_stun_msg_decode(pj_pool_t *pool,
                                       const pj_uint8_t *pdu,
                                       pj_size_t pdu_len,
                                       unsigned options,
                                       pj_stun_msg **p_msg,
                                       pj_size_t *p_parsed_len,
                                       pj_stun_msg **p_response)
{
    pj_stun_msg *msg;
    const pj_uint8_t *start_pdu = pdu;
    pj_bool_t has_msg_int = PJ_FALSE;
    pj_bool_t has_fingerprint = PJ_FALSE;
    pj_status_t status;
    PJ_UNUSED_ARG(options);
    PJ_ASSERT_RETURN(pool && pdu && pdu_len && p_msg, PJ_EINVAL);
    PJ_ASSERT_RETURN(sizeof(pj_stun_msg_hdr) == 20, PJ_EBUG);
    if (p_parsed_len)
        *p_parsed_len = 0;
    if (p_response)
        *p_response = NULL;
    if (options & PJ_STUN_CHECK_PACKET) {
        status = pj_stun_msg_check(pdu, pdu_len, options);
        if (status != PJ_SUCCESS)
            return status;
    }
    msg = PJ_POOL_ZALLOC_T(pool, pj_stun_msg);
    pj_memcpy(&msg->hdr, pdu, sizeof(pj_stun_msg_hdr));
    msg->hdr.type = pj_ntohs(msg->hdr.type);
    msg->hdr.length = pj_ntohs(msg->hdr.length);
    msg->hdr.magic = pj_ntohl(msg->hdr.magic);
    pdu += sizeof(pj_stun_msg_hdr);
    pdu_len = msg->hdr.length;
    if (!PJ_STUN_IS_REQUEST(msg->hdr.type))
        p_response = NULL;
    while (pdu_len >= 4) {
        unsigned attr_type, attr_val_len;
        const struct attr_desc *adesc;
        attr_type = GETVAL16H(pdu, 0);
        attr_val_len = GETVAL16H(pdu, 2);
        attr_val_len = (attr_val_len + 3) & (~3);
        if (pdu_len < attr_val_len) {
            pj_str_t err_msg;
            char err_msg_buf[80];
            err_msg.ptr = err_msg_buf;
            err_msg.slen = pj_ansi_snprintf(err_msg_buf, sizeof(err_msg_buf),
                                            "Attribute %s has invalid length",
                                            pj_stun_get_attr_name(attr_type));
            PJ_LOG(4,(THIS_FILE, "Error decoding message: %.*s",
                      (int)err_msg.slen, err_msg.ptr));
            if (p_response) {
                pj_stun_msg_create_response(pool, msg, 
                                            PJ_STUN_SC_BAD_REQUEST, 
                                            &err_msg, p_response);
            }
            return PJNATH_ESTUNINATTRLEN;
        }
        adesc = find_attr_desc(attr_type);
        if (adesc == NULL) {
            pj_stun_binary_attr *attr = NULL;
            PJ_LOG(5,(THIS_FILE, "Unrecognized attribute type 0x%x", 
                      attr_type));
            if (attr_type <= 0x7FFF) {
                if (p_response) {
                    unsigned err_code = PJ_STUN_SC_UNKNOWN_ATTRIBUTE;
                    status = pj_stun_msg_create_response(pool, msg,
                                                         err_code, NULL, 
                                                         p_response);
                    if (status==PJ_SUCCESS) {
                        pj_uint16_t d = (pj_uint16_t)attr_type;
                        pj_stun_msg_add_unknown_attr(pool, *p_response, 1, &d);
                    }
                }
                return PJ_STATUS_FROM_STUN_CODE(PJ_STUN_SC_UNKNOWN_ATTRIBUTE);
            }
            if (msg->attr_count >= PJ_STUN_MAX_ATTR) {
                if (p_response) {
                    pj_stun_msg_create_response(pool, msg,
                                                PJ_STUN_SC_SERVER_ERROR,
                                                NULL, p_response);
                }
                return PJNATH_ESTUNTOOMANYATTR;
            }
            status = pj_stun_binary_attr_create(pool, attr_type, pdu+4, 
                                                GETVAL16H(pdu, 2), &attr);
            if (status != PJ_SUCCESS) {
                if (p_response) {
                    pj_stun_msg_create_response(pool, msg,
                                                PJ_STUN_SC_SERVER_ERROR,
                                                NULL, p_response);
                }
                PJ_LOG(4,(THIS_FILE, 
                          "Error parsing unknown STUN attribute type %d",
                          attr_type));
                return status;
            }
            msg->attr[msg->attr_count++] = &attr->hdr;
        } else {
            void *attr;
            char err_msg1[PJ_ERR_MSG_SIZE],
                 err_msg2[PJ_ERR_MSG_SIZE];
            status = (adesc->decode_attr)(pool, pdu, &msg->hdr, &attr);
            if (status != PJ_SUCCESS) {
                pj_strerror(status, err_msg1, sizeof(err_msg1));
                if (p_response) {
                    pj_str_t e;
                    e.ptr = err_msg2;
                    e.slen= pj_ansi_snprintf(err_msg2, sizeof(err_msg2),
                                             "%s in %s",
                                             err_msg1,
                                             pj_stun_get_attr_name(attr_type));
                    if (e.slen < 1 || e.slen >= (int)sizeof(err_msg2))
                        e.slen = sizeof(err_msg2) - 1;
                    pj_stun_msg_create_response(pool, msg,
                                                PJ_STUN_SC_BAD_REQUEST,
                                                &e, p_response);
                }
                PJ_LOG(4,(THIS_FILE, 
                          "Error parsing STUN attribute %s: %s",
                          pj_stun_get_attr_name(attr_type), 
                          err_msg1));
                return status;
            }
            if (attr_type == PJ_STUN_ATTR_MESSAGE_INTEGRITY && 
                !has_fingerprint) 
            {
                if (has_msg_int) {
                    if (p_response) {
                        pj_stun_msg_create_response(pool, msg,
                                                    PJ_STUN_SC_BAD_REQUEST,
                                                    NULL, p_response);
                    }
                    return PJNATH_ESTUNDUPATTR;
                }
                has_msg_int = PJ_TRUE;
            } else if (attr_type == PJ_STUN_ATTR_FINGERPRINT) {
                if (has_fingerprint) {
                    if (p_response) {
                        pj_stun_msg_create_response(pool, msg,
                                                    PJ_STUN_SC_BAD_REQUEST,
                                                    NULL, p_response);
                    }
                    return PJNATH_ESTUNDUPATTR;
                }
                has_fingerprint = PJ_TRUE;
            } else {
                if (has_fingerprint) {
                    if (p_response) {
                        pj_stun_msg_create_response(pool, msg,
                                                    PJ_STUN_SC_BAD_REQUEST,
                                                    NULL, p_response);
                    }
                    return PJNATH_ESTUNFINGERPOS;
                }
            }
            if (msg->attr_count >= PJ_STUN_MAX_ATTR) {
                if (p_response) {
                    pj_stun_msg_create_response(pool, msg,
                                                PJ_STUN_SC_SERVER_ERROR,
                                                NULL, p_response);
                }
                return PJNATH_ESTUNTOOMANYATTR;
            }
            msg->attr[msg->attr_count++] = (pj_stun_attr_hdr*)attr;
        }
        if (attr_val_len + 4 >= pdu_len) {
            pdu += pdu_len;
            pdu_len = 0;
        } else {
            pdu += (attr_val_len + 4);
            pdu_len -= (attr_val_len + 4);
        }
    }
    if (pdu_len > 0) {
        PJ_LOG(4,(THIS_FILE, 
                  "Error decoding STUN message: unparsed trailing %d bytes",
                  pdu_len));
        return PJNATH_EINSTUNMSGLEN;
    }
    *p_msg = msg;
    if (p_parsed_len)
        *p_parsed_len = (pdu - start_pdu);
    return PJ_SUCCESS;
}