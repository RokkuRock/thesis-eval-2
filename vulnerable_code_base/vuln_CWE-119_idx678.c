int wvlan_uil_put_info(struct uilreq *urq, struct wl_private *lp)
{
	int                     result = 0;
	ltv_t                   *pLtv;
	bool_t                  ltvAllocated = FALSE;
	ENCSTRCT                sEncryption;
#ifdef USE_WDS
	hcf_16                  hcfPort  = HCF_PORT_0;
#endif   
	DBG_FUNC("wvlan_uil_put_info");
	DBG_ENTER(DbgInfo);
	if (urq->hcfCtx == &(lp->hcfCtx)) {
		if (capable(CAP_NET_ADMIN)) {
			if ((urq->data != NULL) && (urq->len != 0)) {
				if (urq->len < (sizeof(hcf_16) * 2)) {
					urq->len = sizeof(lp->ltvRecord);
					urq->result = UIL_ERR_LEN;
					DBG_ERROR(DbgInfo, "No Length/Type in LTV!!!\n");
					DBG_ERROR(DbgInfo, "UIL_ERR_LEN\n");
					DBG_LEAVE(DbgInfo);
					return result;
				}
				result = verify_area(VERIFY_READ, urq->data, urq->len);
				if (result != 0) {
					urq->result = UIL_FAILURE;
					DBG_ERROR(DbgInfo, "verify_area(), VERIFY_READ FAILED\n");
					DBG_LEAVE(DbgInfo);
					return result;
				}
				copy_from_user(&(lp->ltvRecord), urq->data, sizeof(hcf_16) * 2);
				if (((lp->ltvRecord.len + 1) * sizeof(hcf_16)) > urq->len) {
					urq->len = sizeof(lp->ltvRecord);
					urq->result = UIL_ERR_LEN;
					DBG_ERROR(DbgInfo, "UIL_ERR_LEN\n");
					DBG_LEAVE(DbgInfo);
					return result;
				}
				if (urq->len > sizeof(lp->ltvRecord)) {
					pLtv = kmalloc(urq->len, GFP_KERNEL);
					if (pLtv != NULL) {
						ltvAllocated = TRUE;
					} else {
						DBG_ERROR(DbgInfo, "Alloc FAILED\n");
						urq->len = sizeof(lp->ltvRecord);
						urq->result = UIL_ERR_LEN;
						result = -ENOMEM;
						DBG_LEAVE(DbgInfo);
						return result;
					}
				} else {
					pLtv = &(lp->ltvRecord);
				}
				copy_from_user(pLtv, urq->data, urq->len);
				switch (pLtv->typ) {
				case CFG_CNF_PORT_TYPE:
					lp->PortType    = pLtv->u.u16[0];
					pLtv->u.u16[0]  = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_CNF_OWN_MAC_ADDR:
					break;
				case CFG_CNF_OWN_CHANNEL:
					lp->Channel     = pLtv->u.u16[0];
					pLtv->u.u16[0]  = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_CNF_OWN_ATIM_WINDOW:
					lp->atimWindow  = pLtv->u.u16[0];
					pLtv->u.u16[0]  = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_CNF_SYSTEM_SCALE:
					lp->DistanceBetweenAPs  = pLtv->u.u16[0];
					pLtv->u.u16[0]          = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
				case CFG_CNF_MAX_DATA_LEN:
					break;
				case CFG_CNF_PM_ENABLED:
					lp->PMEnabled   = pLtv->u.u16[0];
					pLtv->u.u16[0]  = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_CNF_MCAST_RX:
					lp->MulticastReceive    = pLtv->u.u16[0];
					pLtv->u.u16[0]          = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_CNF_MAX_SLEEP_DURATION:
					lp->MaxSleepDuration    = pLtv->u.u16[0];
					pLtv->u.u16[0]          = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_CNF_HOLDOVER_DURATION:
					lp->holdoverDuration    = pLtv->u.u16[0];
					pLtv->u.u16[0]          = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_CNF_OWN_NAME:
					memset(lp->StationName, 0, sizeof(lp->StationName));
					memcpy((void *)lp->StationName, (void *)&pLtv->u.u8[2], (size_t)pLtv->u.u16[0]);
					pLtv->u.u16[0] = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_CNF_LOAD_BALANCING:
					lp->loadBalancing       = pLtv->u.u16[0];
					pLtv->u.u16[0]          = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_CNF_MEDIUM_DISTRIBUTION:
					lp->mediumDistribution  = pLtv->u.u16[0];
					pLtv->u.u16[0]          = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
#ifdef WARP
				case CFG_CNF_TX_POW_LVL:
					lp->txPowLevel          = pLtv->u.u16[0];
					pLtv->u.u16[0]          = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_SUPPORTED_RATE_SET_CNTL:         
					lp->srsc[0]             = pLtv->u.u16[0];
					lp->srsc[1]             = pLtv->u.u16[1];
					pLtv->u.u16[0]          = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					pLtv->u.u16[1]          = CNV_INT_TO_LITTLE(pLtv->u.u16[1]);
					break;
				case CFG_BASIC_RATE_SET_CNTL:         
					lp->brsc[0]             = pLtv->u.u16[0];
					lp->brsc[1]             = pLtv->u.u16[1];
					pLtv->u.u16[0]          = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					pLtv->u.u16[1]          = CNV_INT_TO_LITTLE(pLtv->u.u16[1]);
					break;
				case CFG_CNF_CONNECTION_CNTL:
					lp->connectionControl   = pLtv->u.u16[0];
					pLtv->u.u16[0]          = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
#endif   
#if 1  
				case CFG_CNF_OWN_DTIM_PERIOD:
					lp->DTIMPeriod  = pLtv->u.u16[0];
					pLtv->u.u16[0]  = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
#ifdef WARP
				case CFG_CNF_OWN_BEACON_INTERVAL:         
					lp->ownBeaconInterval   = pLtv->u.u16[0];
					pLtv->u.u16[0]          = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
#endif  
				case CFG_COEXISTENSE_BEHAVIOUR:          
					lp->coexistence         = pLtv->u.u16[0];
					pLtv->u.u16[0]          = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
#ifdef USE_WDS
				case CFG_CNF_WDS_ADDR1:
					memcpy(&lp->wds_port[0].wdsAddress, &pLtv->u.u8[0], ETH_ALEN);
					hcfPort = HCF_PORT_1;
					break;
				case CFG_CNF_WDS_ADDR2:
					memcpy(&lp->wds_port[1].wdsAddress, &pLtv->u.u8[0], ETH_ALEN);
					hcfPort = HCF_PORT_2;
					break;
				case CFG_CNF_WDS_ADDR3:
					memcpy(&lp->wds_port[2].wdsAddress, &pLtv->u.u8[0], ETH_ALEN);
					hcfPort = HCF_PORT_3;
					break;
				case CFG_CNF_WDS_ADDR4:
					memcpy(&lp->wds_port[3].wdsAddress, &pLtv->u.u8[0], ETH_ALEN);
					hcfPort = HCF_PORT_4;
					break;
				case CFG_CNF_WDS_ADDR5:
					memcpy(&lp->wds_port[4].wdsAddress, &pLtv->u.u8[0], ETH_ALEN);
					hcfPort = HCF_PORT_5;
					break;
				case CFG_CNF_WDS_ADDR6:
					memcpy(&lp->wds_port[5].wdsAddress, &pLtv->u.u8[0], ETH_ALEN);
					hcfPort = HCF_PORT_6;
					break;
#endif   
				case CFG_CNF_MCAST_PM_BUF:
					lp->multicastPMBuffering    = pLtv->u.u16[0];
					pLtv->u.u16[0]              = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_CNF_REJECT_ANY:
					lp->RejectAny   = pLtv->u.u16[0];
					pLtv->u.u16[0]  = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
#endif
				case CFG_CNF_ENCRYPTION:
					lp->EnableEncryption    = pLtv->u.u16[0];
					pLtv->u.u16[0]          = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_CNF_AUTHENTICATION:
					lp->authentication  = pLtv->u.u16[0];
					pLtv->u.u16[0]      = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
#if 1  
				case CFG_CNF_MCAST_RATE:
					break;
				case CFG_CNF_INTRA_BSS_RELAY:
					lp->intraBSSRelay   = pLtv->u.u16[0];
					pLtv->u.u16[0]      = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
#endif
				case CFG_CNF_MICRO_WAVE:
					break;
				case CFG_CNF_OWN_SSID:
				case CFG_DESIRED_SSID:
					memset(lp->NetworkName, 0, sizeof(lp->NetworkName));
					memcpy((void *)lp->NetworkName, (void *)&pLtv->u.u8[2], (size_t)pLtv->u.u16[0]);
					pLtv->u.u16[0] = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					if ((strlen(&pLtv->u.u8[2]) == 0) ||
					   (strcmp(&pLtv->u.u8[2], "ANY") == 0) ||
					   (strcmp(&pLtv->u.u8[2], "any") == 0)) {
						pLtv->u.u16[0] = 0;
						pLtv->u.u8[2]  = 0;
					}
					break;
				case CFG_GROUP_ADDR:
					break;
				case CFG_CREATE_IBSS:
					lp->CreateIBSS  = pLtv->u.u16[0];
					pLtv->u.u16[0]  = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_RTS_THRH:
					lp->RTSThreshold    = pLtv->u.u16[0];
					pLtv->u.u16[0]      = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_TX_RATE_CNTL:
					lp->TxRateControl[0]    = pLtv->u.u16[0];
					lp->TxRateControl[1]    = pLtv->u.u16[1];
					pLtv->u.u16[0]          = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					pLtv->u.u16[1]          = CNV_INT_TO_LITTLE(pLtv->u.u16[1]);
					break;
				case CFG_PROMISCUOUS_MODE:
					break;
#if 1  
				case CFG_RTS_THRH0:
					lp->RTSThreshold    = pLtv->u.u16[0];
					pLtv->u.u16[0]      = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_TX_RATE_CNTL0:
					pLtv->u.u16[0]      = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
#ifdef USE_WDS
				case CFG_RTS_THRH1:
					lp->wds_port[0].rtsThreshold    = pLtv->u.u16[0];
					pLtv->u.u16[0]                  = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					hcfPort                         = HCF_PORT_1;
					break;
				case CFG_RTS_THRH2:
					lp->wds_port[1].rtsThreshold    = pLtv->u.u16[0];
					pLtv->u.u16[0]                  = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					hcfPort                         = HCF_PORT_2;
					break;
				case CFG_RTS_THRH3:
					lp->wds_port[2].rtsThreshold    = pLtv->u.u16[0];
					pLtv->u.u16[0]                  = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					hcfPort                         = HCF_PORT_3;
					break;
				case CFG_RTS_THRH4:
					lp->wds_port[3].rtsThreshold    = pLtv->u.u16[0];
					pLtv->u.u16[0]                  = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					hcfPort                         = HCF_PORT_4;
					break;
				case CFG_RTS_THRH5:
					lp->wds_port[4].rtsThreshold    = pLtv->u.u16[0];
					pLtv->u.u16[0]                  = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					hcfPort                         = HCF_PORT_5;
					break;
				case CFG_RTS_THRH6:
					lp->wds_port[5].rtsThreshold    = pLtv->u.u16[0];
					pLtv->u.u16[0]                  = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					hcfPort                         = HCF_PORT_6;
					break;
				case CFG_TX_RATE_CNTL1:
					lp->wds_port[0].txRateCntl  = pLtv->u.u16[0];
					pLtv->u.u16[0]              = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					hcfPort                     = HCF_PORT_1;
					break;
				case CFG_TX_RATE_CNTL2:
					lp->wds_port[1].txRateCntl  = pLtv->u.u16[0];
					pLtv->u.u16[0]              = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					hcfPort                     = HCF_PORT_2;
					break;
				case CFG_TX_RATE_CNTL3:
					lp->wds_port[2].txRateCntl  = pLtv->u.u16[0];
					pLtv->u.u16[0]              = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					hcfPort                     = HCF_PORT_3;
					break;
				case CFG_TX_RATE_CNTL4:
					lp->wds_port[3].txRateCntl  = pLtv->u.u16[0];
					pLtv->u.u16[0]              = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					hcfPort                     = HCF_PORT_4;
					break;
				case CFG_TX_RATE_CNTL5:
					lp->wds_port[4].txRateCntl  = pLtv->u.u16[0];
					pLtv->u.u16[0]              = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					hcfPort                     = HCF_PORT_5;
					break;
				case CFG_TX_RATE_CNTL6:
					lp->wds_port[5].txRateCntl  = pLtv->u.u16[0];
					pLtv->u.u16[0]              = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					hcfPort                     = HCF_PORT_6;
					break;
#endif   
#endif   
				case CFG_DEFAULT_KEYS:
					{
						CFG_DEFAULT_KEYS_STRCT *pKeys = (CFG_DEFAULT_KEYS_STRCT *)pLtv;
						pKeys->key[0].len = CNV_INT_TO_LITTLE(pKeys->key[0].len);
						pKeys->key[1].len = CNV_INT_TO_LITTLE(pKeys->key[1].len);
						pKeys->key[2].len = CNV_INT_TO_LITTLE(pKeys->key[2].len);
						pKeys->key[3].len = CNV_INT_TO_LITTLE(pKeys->key[3].len);
						memcpy((void *)&(lp->DefaultKeys), (void *)pKeys,
								sizeof(CFG_DEFAULT_KEYS_STRCT));
					}
					break;
				case CFG_TX_KEY_ID:
					lp->TransmitKeyID   = pLtv->u.u16[0];
					pLtv->u.u16[0]      = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_SCAN_SSID:
					break;
				case CFG_TICK_TIME:
					break;
				case CFG_MAX_LOAD_TIME:
				case CFG_DL_BUF:
				case CFG_NIC_SERIAL_NUMBER:
				case CFG_NIC_IDENTITY:
				case CFG_NIC_MFI_SUP_RANGE:
				case CFG_NIC_CFI_SUP_RANGE:
				case CFG_NIC_TEMP_TYPE:
				case CFG_NIC_PROFILE:
				case CFG_FW_IDENTITY:
				case CFG_FW_SUP_RANGE:
				case CFG_MFI_ACT_RANGES_STA:
				case CFG_CFI_ACT_RANGES_STA:
				case CFG_PORT_STAT:
				case CFG_CUR_SSID:
				case CFG_CUR_BSSID:
				case CFG_COMMS_QUALITY:
				case CFG_CUR_TX_RATE:
				case CFG_CUR_BEACON_INTERVAL:
				case CFG_CUR_SCALE_THRH:
				case CFG_PROTOCOL_RSP_TIME:
				case CFG_CUR_SHORT_RETRY_LIMIT:
				case CFG_CUR_LONG_RETRY_LIMIT:
				case CFG_MAX_TX_LIFETIME:
				case CFG_MAX_RX_LIFETIME:
				case CFG_CF_POLLABLE:
				case CFG_AUTHENTICATION_ALGORITHMS:
				case CFG_PRIVACY_OPT_IMPLEMENTED:
				case CFG_NIC_MAC_ADDR:
				case CFG_PCF_INFO:
				case CFG_PHY_TYPE:
				case CFG_CUR_CHANNEL:
				case CFG_SUPPORTED_DATA_RATES:
					break;
				case CFG_AP_MODE:
					DBG_ERROR(DbgInfo, "set CFG_AP_MODE no longer supported\n");
					break;
				case CFG_ENCRYPT_STRING:
					memset(lp->szEncryption, 0, sizeof(lp->szEncryption));
					memcpy((void *)lp->szEncryption,  (void *)&pLtv->u.u8[0],
							(pLtv->len * sizeof(hcf_16)));
					wl_wep_decode(CRYPT_CODE, &sEncryption,
								    lp->szEncryption);
					lp->TransmitKeyID    = sEncryption.wTxKeyID + 1;
					lp->EnableEncryption = sEncryption.wEnabled;
					memcpy(&lp->DefaultKeys, &sEncryption.EncStr,
							sizeof(CFG_DEFAULT_KEYS_STRCT));
					break;
				case CFG_DRIVER_ENABLE:
					lp->driverEnable    = pLtv->u.u16[0];
					pLtv->u.u16[0]      = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_WOLAS_ENABLE:
					lp->wolasEnable = pLtv->u.u16[0];
					pLtv->u.u16[0]  = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_SET_WPA_AUTH_KEY_MGMT_SUITE:
					lp->AuthKeyMgmtSuite = pLtv->u.u16[0];
					pLtv->u.u16[0]  = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_DISASSOCIATE_ADDR:
					pLtv->u.u16[ETH_ALEN / 2] = CNV_INT_TO_LITTLE(pLtv->u.u16[ETH_ALEN / 2]);
					break;
				case CFG_ADD_TKIP_DEFAULT_KEY:
				case CFG_REMOVE_TKIP_DEFAULT_KEY:
					pLtv->u.u16[0] = CNV_INT_TO_LITTLE(pLtv->u.u16[0]);
					break;
				case CFG_ADD_TKIP_MAPPED_KEY:
					break;
				case CFG_REMOVE_TKIP_MAPPED_KEY:
					break;
				case CFG_MB_INFO:
				case CFG_IFB:
				default:
					break;
				}
				switch (pLtv->typ) {
				case CFG_CNF_PORT_TYPE:
				case CFG_CNF_OWN_MAC_ADDR:
				case CFG_CNF_OWN_CHANNEL:
				case CFG_CNF_OWN_SSID:
				case CFG_CNF_OWN_ATIM_WINDOW:
				case CFG_CNF_SYSTEM_SCALE:
				case CFG_CNF_MAX_DATA_LEN:
				case CFG_CNF_PM_ENABLED:
				case CFG_CNF_MCAST_RX:
				case CFG_CNF_MAX_SLEEP_DURATION:
				case CFG_CNF_HOLDOVER_DURATION:
				case CFG_CNF_OWN_NAME:
				case CFG_CNF_LOAD_BALANCING:
				case CFG_CNF_MEDIUM_DISTRIBUTION:
#ifdef WARP
				case CFG_CNF_TX_POW_LVL:
				case CFG_CNF_CONNECTION_CNTL:
#endif  
#if 1  
				case CFG_CNF_OWN_DTIM_PERIOD:
#ifdef WARP
				case CFG_CNF_OWN_BEACON_INTERVAL:                     
#endif  
#ifdef USE_WDS
				case CFG_CNF_WDS_ADDR1:
				case CFG_CNF_WDS_ADDR2:
				case CFG_CNF_WDS_ADDR3:
				case CFG_CNF_WDS_ADDR4:
				case CFG_CNF_WDS_ADDR5:
				case CFG_CNF_WDS_ADDR6:
#endif
				case CFG_CNF_MCAST_PM_BUF:
				case CFG_CNF_REJECT_ANY:
#endif
				case CFG_CNF_ENCRYPTION:
				case CFG_CNF_AUTHENTICATION:
#if 1  
				case CFG_CNF_EXCL_UNENCRYPTED:
				case CFG_CNF_MCAST_RATE:
				case CFG_CNF_INTRA_BSS_RELAY:
#endif
				case CFG_CNF_MICRO_WAVE:
				case CFG_AP_MODE:
				case CFG_ENCRYPT_STRING:
				case CFG_WOLAS_ENABLE:
				case CFG_MB_INFO:
				case CFG_IFB:
					break;
				case CFG_DRIVER_ENABLE:
					if (lp->driverEnable) {
						hcf_cntl(&(lp->hcfCtx), HCF_CNTL_ENABLE | HCF_PORT_0);
						hcf_cntl(&(lp->hcfCtx), HCF_CNTL_CONNECT);
					} else {
						hcf_cntl(&(lp->hcfCtx), HCF_CNTL_DISABLE | HCF_PORT_0);
						hcf_cntl(&(lp->hcfCtx), HCF_CNTL_DISCONNECT);
					}
					break;
				default:
					wl_act_int_off(lp);
					urq->result = hcf_put_info(&(lp->hcfCtx), (LTVP) pLtv);
					wl_act_int_on(lp);
					break;
				}
				if (ltvAllocated)
					kfree(pLtv);
			} else {
				urq->result = UIL_FAILURE;
			}
		} else {
			DBG_ERROR(DbgInfo, "EPERM\n");
			urq->result = UIL_FAILURE;
			result = -EPERM;
		}
	} else {
		DBG_ERROR(DbgInfo, "UIL_ERR_WRONG_IFB\n");
		urq->result = UIL_ERR_WRONG_IFB;
	}
	DBG_LEAVE(DbgInfo);
	return result;
}  