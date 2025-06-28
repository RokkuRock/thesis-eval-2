AirPDcapDecryptWPABroadcastKey(const EAPOL_RSN_KEY *pEAPKey, guint8 *decryption_key, PAIRPDCAP_SEC_ASSOCIATION sa, guint eapol_len)
{
    guint8 key_version;
    guint8 *key_data;
    guint8  *szEncryptedKey;
    guint16 key_bytes_len = 0;  
    guint16 key_len;            
    static AIRPDCAP_KEY_ITEM dummy_key;  
    AIRPDCAP_SEC_ASSOCIATION *tmp_sa;
    key_version = AIRPDCAP_EAP_KEY_DESCR_VER(pEAPKey->key_information[1]);
    if (key_version == AIRPDCAP_WPA_KEY_VER_NOT_CCMP){
        key_bytes_len = pntoh16(pEAPKey->key_length);
    }else if (key_version == AIRPDCAP_WPA_KEY_VER_AES_CCMP){
        key_bytes_len = pntoh16(pEAPKey->key_data_len);
        if (key_bytes_len < 16) {
            return AIRPDCAP_RET_NO_VALID_HANDSHAKE;
        }
    }
    if (key_bytes_len < GROUP_KEY_MIN_LEN || key_bytes_len > eapol_len - sizeof(EAPOL_RSN_KEY)) {
        return AIRPDCAP_RET_NO_VALID_HANDSHAKE;
    }
    key_data = (guint8 *)pEAPKey + sizeof(EAPOL_RSN_KEY);
    szEncryptedKey = (guint8 *)g_memdup(key_data, key_bytes_len);
    DEBUG_DUMP("Encrypted Broadcast key:", szEncryptedKey, key_bytes_len);
    DEBUG_DUMP("KeyIV:", pEAPKey->key_iv, 16);
    DEBUG_DUMP("decryption_key:", decryption_key, 16);
    tmp_sa=(AIRPDCAP_SEC_ASSOCIATION *)g_malloc(sizeof(AIRPDCAP_SEC_ASSOCIATION));
    memcpy(tmp_sa, sa, sizeof(AIRPDCAP_SEC_ASSOCIATION));
    sa->next=tmp_sa;
    if (key_version == AIRPDCAP_WPA_KEY_VER_NOT_CCMP){
        guint8 new_key[32];
        guint8 dummy[256];
        rc4_state_struct rc4_state;
        sa->wpa.key_ver = (key_bytes_len >=TKIP_GROUP_KEY_LEN)?AIRPDCAP_WPA_KEY_VER_NOT_CCMP:AIRPDCAP_WPA_KEY_VER_AES_CCMP;
        memcpy(new_key, pEAPKey->key_iv, 16);
        memcpy(new_key+16, decryption_key, 16);
        DEBUG_DUMP("FullDecrKey:", new_key, 32);
        crypt_rc4_init(&rc4_state, new_key, sizeof(new_key));
        crypt_rc4(&rc4_state, dummy, 256);
        crypt_rc4(&rc4_state, szEncryptedKey, key_bytes_len);
    } else if (key_version == AIRPDCAP_WPA_KEY_VER_AES_CCMP){
        guint8 key_found;
        guint8 key_length;
        guint16 key_index;
        guint8 *decrypted_data;
        decrypted_data = AES_unwrap(decryption_key, 16, szEncryptedKey,  key_bytes_len);
        key_found = FALSE;
        key_index = 0;
        while(key_index < (key_bytes_len - 6) && !key_found){
            guint8 rsn_id;
            guint32 type;
            rsn_id = decrypted_data[key_index];
            type = ((decrypted_data[key_index + 2] << 24) +
                    (decrypted_data[key_index + 3] << 16) +
                    (decrypted_data[key_index + 4] << 8) +
                     (decrypted_data[key_index + 5]));
            if (rsn_id == 0xdd && type == 0x000fac01) {
                key_found = TRUE;
            } else {
                key_index += decrypted_data[key_index+1]+2;
            }
        }
        if (key_found){
            key_length = decrypted_data[key_index+1] - 6;
            if (key_index+8 >= key_bytes_len ||
                key_length > key_bytes_len - key_index - 8) {
                g_free(decrypted_data);
                g_free(szEncryptedKey);
                return AIRPDCAP_RET_NO_VALID_HANDSHAKE;
            }
            memcpy(szEncryptedKey, decrypted_data+key_index+8, key_length);
        } else {
            g_free(decrypted_data);
            g_free(szEncryptedKey);
            return AIRPDCAP_RET_NO_VALID_HANDSHAKE;
        }
        if (key_length == TKIP_GROUP_KEY_LEN)
            sa->wpa.key_ver = AIRPDCAP_WPA_KEY_VER_NOT_CCMP;
        else
            sa->wpa.key_ver = AIRPDCAP_WPA_KEY_VER_AES_CCMP;
        g_free(decrypted_data);
    }
    key_len = (sa->wpa.key_ver==AIRPDCAP_WPA_KEY_VER_NOT_CCMP)?TKIP_GROUP_KEY_LEN:CCMP_GROUP_KEY_LEN;
    if (key_len > key_bytes_len) {
        g_free(szEncryptedKey);
        return AIRPDCAP_RET_NO_VALID_HANDSHAKE;
    }
    DEBUG_DUMP("Broadcast key:", szEncryptedKey, key_len);
    sa->key = &dummy_key;   
    sa->validKey = TRUE;
    memset(sa->wpa.ptk, 0, sizeof(sa->wpa.ptk));
    memcpy(sa->wpa.ptk+32, szEncryptedKey, key_len);
    g_free(szEncryptedKey);
    return AIRPDCAP_RET_SUCCESS_HANDSHAKE;
}