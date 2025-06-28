static INT AirPDcapScanForKeys(
    PAIRPDCAP_CONTEXT ctx,
    const guint8 *data,
    const guint mac_header_len,
    const guint tot_len,
    AIRPDCAP_SEC_ASSOCIATION_ID id
)
{
    const UCHAR *addr;
    guint bodyLength;
    PAIRPDCAP_SEC_ASSOCIATION sta_sa;
    PAIRPDCAP_SEC_ASSOCIATION sa;
    guint offset = 0;
    const guint8 dot1x_header[] = {
        0xAA,              
        0xAA,              
        0x03,              
        0x00, 0x00, 0x00,  
        0x88, 0x8E         
    };
    const guint8 bt_dot1x_header[] = {
        0xAA,              
        0xAA,              
        0x03,              
        0x00, 0x19, 0x58,  
        0x00, 0x03         
    };
    const guint8 tdls_header[] = {
        0xAA,              
        0xAA,              
        0x03,              
        0x00, 0x00, 0x00,  
        0x89, 0x0D,        
        0x02,              
        0X0C               
    };
    const EAPOL_RSN_KEY *pEAPKey;
#ifdef _DEBUG
#define MSGBUF_LEN 255
    CHAR msgbuf[MSGBUF_LEN];
#endif
    AIRPDCAP_DEBUG_TRACE_START("AirPDcapScanForKeys");
    offset = mac_header_len;
    if (memcmp(data+offset, dot1x_header, 8) == 0 || memcmp(data+offset, bt_dot1x_header, 8) == 0) {
        AIRPDCAP_DEBUG_PRINT_LINE("AirPDcapScanForKeys", "Authentication: EAPOL packet", AIRPDCAP_DEBUG_LEVEL_3);
        offset+=8;
        if (data[offset+1]!=3) {
            AIRPDCAP_DEBUG_PRINT_LINE("AirPDcapScanForKeys", "Not EAPOL-Key", AIRPDCAP_DEBUG_LEVEL_3);
            return AIRPDCAP_RET_NO_VALID_HANDSHAKE;
        }
        bodyLength=pntoh16(data+offset+2);
        if ((tot_len-offset-4) < bodyLength) {  
            AIRPDCAP_DEBUG_PRINT_LINE("AirPDcapScanForKeys", "EAPOL body too short", AIRPDCAP_DEBUG_LEVEL_3);
            return AIRPDCAP_RET_NO_VALID_HANDSHAKE;
        }
        offset+=4;
        pEAPKey = (const EAPOL_RSN_KEY *) (data+offset);
        if (   
            pEAPKey->type != AIRPDCAP_RSN_WPA2_KEY_DESCRIPTOR &&              
            pEAPKey->type != AIRPDCAP_RSN_WPA_KEY_DESCRIPTOR)            
        {
            AIRPDCAP_DEBUG_PRINT_LINE("AirPDcapScanForKeys", "Not valid key descriptor type", AIRPDCAP_DEBUG_LEVEL_3);
            return AIRPDCAP_RET_NO_VALID_HANDSHAKE;
        }
        offset+=1;
        sa = AirPDcapGetSaPtr(ctx, &id);
        if (sa == NULL){
            AIRPDCAP_DEBUG_PRINT_LINE("AirPDcapScanForKeys", "No SA for BSSID found", AIRPDCAP_DEBUG_LEVEL_3);
            return AIRPDCAP_RET_REQ_DATA;
        }
        if (AirPDcapRsna4WHandshake(ctx, data, sa, offset, tot_len) == AIRPDCAP_RET_SUCCESS_HANDSHAKE)
            return AIRPDCAP_RET_SUCCESS_HANDSHAKE;
        if (mac_header_len + GROUP_KEY_PAYLOAD_LEN_MIN > tot_len) {
            AIRPDCAP_DEBUG_PRINT_LINE("AirPDcapScanForKeys", "Message too short for Group Key", AIRPDCAP_DEBUG_LEVEL_3);
            return AIRPDCAP_RET_NO_VALID_HANDSHAKE;
        }
        if (AIRPDCAP_EAP_KEY(data[offset+1])!=0 ||
            AIRPDCAP_EAP_ACK(data[offset+1])!=1 ||
            AIRPDCAP_EAP_MIC(data[offset]) != 1 ||
            AIRPDCAP_EAP_SEC(data[offset]) != 1){
            AIRPDCAP_DEBUG_PRINT_LINE("AirPDcapScanForKeys", "Key bitfields not correct for Group Key", AIRPDCAP_DEBUG_LEVEL_3);
            return AIRPDCAP_RET_NO_VALID_HANDSHAKE;
        }
        memcpy(id.sta, broadcast_mac, AIRPDCAP_MAC_LEN);
        sa = AirPDcapGetSaPtr(ctx, &id);
        if (sa == NULL){
            return AIRPDCAP_RET_REQ_DATA;
        }
        if ( (addr=AirPDcapGetStaAddress((const AIRPDCAP_MAC_FRAME_ADDR4 *)(data))) != NULL) {
            memcpy(id.sta, addr, AIRPDCAP_MAC_LEN);
#ifdef _DEBUG
            g_snprintf(msgbuf, MSGBUF_LEN, "ST_MAC: %2X.%2X.%2X.%2X.%2X.%2X\t", id.sta[0],id.sta[1],id.sta[2],id.sta[3],id.sta[4],id.sta[5]);
#endif
            AIRPDCAP_DEBUG_PRINT_LINE("AirPDcapScanForKeys", msgbuf, AIRPDCAP_DEBUG_LEVEL_3);
        } else {
            AIRPDCAP_DEBUG_PRINT_LINE("AirPDcapScanForKeys", "SA not found", AIRPDCAP_DEBUG_LEVEL_5);
            return AIRPDCAP_RET_REQ_DATA;
        }
        sta_sa = AirPDcapGetSaPtr(ctx, &id);
        if (sta_sa == NULL){
            return AIRPDCAP_RET_REQ_DATA;
        }
        return (AirPDcapDecryptWPABroadcastKey(pEAPKey, sta_sa->wpa.ptk+16, sa, tot_len-offset+1));
    } else if (memcmp(data+offset, tdls_header, 10) == 0) {
        const guint8 *initiator, *responder;
        guint8 action;
        guint status, offset_rsne = 0, offset_fte = 0, offset_link = 0, offset_timeout = 0;
        AIRPDCAP_DEBUG_PRINT_LINE("AirPDcapScanForKeys", "Authentication: TDLS Action Frame", AIRPDCAP_DEBUG_LEVEL_3);
        offset+=10;
        action = data[offset];
        if (action!=1 && action!=2) {
            AIRPDCAP_DEBUG_PRINT_LINE("AirPDcapScanForKeys", "Not Response nor confirm", AIRPDCAP_DEBUG_LEVEL_3);
            return AIRPDCAP_RET_NO_VALID_HANDSHAKE;
        }
        offset++;
        status=pntoh16(data+offset);
        if (status!=0) {
            AIRPDCAP_DEBUG_PRINT_LINE("AirPDcapScanForKeys", "TDLS setup not successfull", AIRPDCAP_DEBUG_LEVEL_3);
            return AIRPDCAP_RET_NO_VALID_HANDSHAKE;
        }
        offset+=5;
        while(offset < (tot_len - 2)) {
            if (data[offset] == 48) {
                offset_rsne = offset;
            } else if (data[offset] == 55) {
                offset_fte = offset;
            } else if (data[offset] == 56) {
                offset_timeout = offset;
            } else if (data[offset] == 101) {
                offset_link = offset;
            }
            if (tot_len < offset + data[offset + 1] + 2) {
                return AIRPDCAP_RET_NO_VALID_HANDSHAKE;
            }
            offset += data[offset + 1] + 2;
        }
        if (offset_rsne == 0 || offset_fte == 0 ||
            offset_timeout == 0 || offset_link == 0)
        {
            AIRPDCAP_DEBUG_PRINT_LINE("AirPDcapScanForKeys", "Cannot Find all necessary IEs", AIRPDCAP_DEBUG_LEVEL_3);
            return AIRPDCAP_RET_NO_VALID_HANDSHAKE;
        }
        AIRPDCAP_DEBUG_PRINT_LINE("AirPDcapScanForKeys", "Found RSNE/Fast BSS/Timeout Interval/Link IEs", AIRPDCAP_DEBUG_LEVEL_3);
        initiator = &data[offset_link + 8];
        responder = &data[offset_link + 14];
        if (memcmp(initiator, responder, AIRPDCAP_MAC_LEN) < 0) {
            memcpy(id.sta, initiator, AIRPDCAP_MAC_LEN);
            memcpy(id.bssid, responder, AIRPDCAP_MAC_LEN);
        } else {
            memcpy(id.sta, responder, AIRPDCAP_MAC_LEN);
            memcpy(id.bssid, initiator, AIRPDCAP_MAC_LEN);
        }
        sa = AirPDcapGetSaPtr(ctx, &id);
        if (sa == NULL){
            return AIRPDCAP_RET_REQ_DATA;
        }
        if (sa->validKey) {
            if (memcmp(sa->wpa.nonce, data + offset_fte + 52, AIRPDCAP_WPA_NONCE_LEN) == 0) {
                return AIRPDCAP_RET_SUCCESS_HANDSHAKE;
            } else {
                AIRPDCAP_SEC_ASSOCIATION *tmp_sa = g_new(AIRPDCAP_SEC_ASSOCIATION, 1);
                memcpy(tmp_sa, sa, sizeof(AIRPDCAP_SEC_ASSOCIATION));
                sa->next=tmp_sa;
                sa->validKey = FALSE;
            }
        }
        if (AirPDcapTDLSDeriveKey(sa, data, offset_rsne, offset_fte, offset_timeout, offset_link, action)
            == AIRPDCAP_RET_SUCCESS) {
            AIRPDCAP_DEBUG_TRACE_END("AirPDcapScanForKeys");
            return AIRPDCAP_RET_SUCCESS_HANDSHAKE;
        }
    } else {
        AIRPDCAP_DEBUG_PRINT_LINE("AirPDcapScanForKeys", "Skipping: not an EAPOL packet", AIRPDCAP_DEBUG_LEVEL_3);
    }
    AIRPDCAP_DEBUG_TRACE_END("AirPDcapScanForKeys");
    return AIRPDCAP_RET_NO_VALID_HANDSHAKE;
}