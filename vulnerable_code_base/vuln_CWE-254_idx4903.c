int bzrtp_packetParser(bzrtpContext_t *zrtpContext, bzrtpChannelContext_t *zrtpChannelContext, const uint8_t * input, uint16_t inputLength, bzrtpPacket_t *zrtpPacket) {
	int i;
	uint8_t *messageContent = (uint8_t *)(input+ZRTP_PACKET_HEADER_LENGTH+ZRTP_MESSAGE_HEADER_LENGTH);
	switch (zrtpPacket->messageType) {
		case MSGTYPE_HELLO : 
			{
				bzrtpHelloMessage_t *messageData;
				messageData = (bzrtpHelloMessage_t *)malloc(sizeof(bzrtpHelloMessage_t));
				memcpy(messageData->version, messageContent, 4);
				messageContent +=4;
				memcpy(messageData->clientIdentifier, messageContent, 16);
				messageContent +=16;
				memcpy(messageData->H3, messageContent, 32);
				messageContent +=32;
				memcpy(messageData->ZID, messageContent, 12);
				messageContent +=12;
				messageData->S = ((*messageContent)>>6)&0x01;
				messageData->M = ((*messageContent)>>5)&0x01;
				messageData->P = ((*messageContent)>>4)&0x01;
				messageContent +=1;
				messageData->hc = MIN((*messageContent)&0x0F, 7);
				messageContent +=1;
				messageData->cc = MIN(((*messageContent)>>4)&0x0F, 7);
				messageData->ac = MIN((*messageContent)&0x0F, 7);
				messageContent +=1;
				messageData->kc = MIN(((*messageContent)>>4)&0x0F, 7);
				messageData->sc = MIN((*messageContent)&0x0F, 7);
				messageContent +=1;
				if (zrtpPacket->messageLength != ZRTP_HELLOMESSAGE_FIXED_LENGTH + 4*((uint16_t)(messageData->hc)+(uint16_t)(messageData->cc)+(uint16_t)(messageData->ac)+(uint16_t)(messageData->kc)+(uint16_t)(messageData->sc))) {
					free(messageData);
					return BZRTP_PARSER_ERROR_INVALIDMESSAGE;
				}
				for (i=0; i<messageData->hc; i++) {
					messageData->supportedHash[i] = cryptoAlgoTypeStringToInt(messageContent, ZRTP_HASH_TYPE);
					messageContent +=4;
				}
				for (i=0; i<messageData->cc; i++) {
					messageData->supportedCipher[i] = cryptoAlgoTypeStringToInt(messageContent, ZRTP_CIPHERBLOCK_TYPE);
					messageContent +=4;
				}
				for (i=0; i<messageData->ac; i++) {
					messageData->supportedAuthTag[i] = cryptoAlgoTypeStringToInt(messageContent, ZRTP_AUTHTAG_TYPE);
					messageContent +=4;
				}
				for (i=0; i<messageData->kc; i++) {
					messageData->supportedKeyAgreement[i] = cryptoAlgoTypeStringToInt(messageContent, ZRTP_KEYAGREEMENT_TYPE);
					messageContent +=4;
				}
				for (i=0; i<messageData->sc; i++) {
					messageData->supportedSas[i] = cryptoAlgoTypeStringToInt(messageContent, ZRTP_SAS_TYPE);
					messageContent +=4;
				}
				addMandatoryCryptoTypesIfNeeded(ZRTP_HASH_TYPE, messageData->supportedHash, &messageData->hc);
				addMandatoryCryptoTypesIfNeeded(ZRTP_CIPHERBLOCK_TYPE, messageData->supportedCipher, &messageData->cc);
				addMandatoryCryptoTypesIfNeeded(ZRTP_AUTHTAG_TYPE, messageData->supportedAuthTag, &messageData->ac);
				addMandatoryCryptoTypesIfNeeded(ZRTP_KEYAGREEMENT_TYPE, messageData->supportedKeyAgreement, &messageData->kc);
				addMandatoryCryptoTypesIfNeeded(ZRTP_SAS_TYPE, messageData->supportedSas, &messageData->sc);
				memcpy(messageData->MAC, messageContent, 8);
				zrtpPacket->messageData = (void *)messageData;
				zrtpPacket->packetString = (uint8_t *)malloc(inputLength*sizeof(uint8_t));
				memcpy(zrtpPacket->packetString, input, inputLength);  
			}
			break;  
		case MSGTYPE_HELLOACK :
			{
				if (zrtpPacket->messageLength != ZRTP_HELLOACKMESSAGE_FIXED_LENGTH) {
					return BZRTP_PARSER_ERROR_INVALIDMESSAGE;
				}
			}
			break;  
		case MSGTYPE_COMMIT:
			{
				uint8_t checkH3[32];
				uint8_t checkMAC[32];
				bzrtpHelloMessage_t *peerHelloMessageData;
				uint16_t variableLength = 0;
				bzrtpCommitMessage_t *messageData;
				messageData = (bzrtpCommitMessage_t *)malloc(sizeof(bzrtpCommitMessage_t));
				memcpy(messageData->H2, messageContent, 32);
				messageContent +=32;
				if (zrtpChannelContext->peerPackets[HELLO_MESSAGE_STORE_ID] == NULL) {
					free (messageData);
					return BZRTP_PARSER_ERROR_UNEXPECTEDMESSAGE;
				}
				peerHelloMessageData = (bzrtpHelloMessage_t *)zrtpChannelContext->peerPackets[HELLO_MESSAGE_STORE_ID]->messageData;
				bctoolbox_sha256(messageData->H2, 32, 32, checkH3);
				if (memcmp(checkH3, peerHelloMessageData->H3, 32) != 0) {
					free (messageData);
					return BZRTP_PARSER_ERROR_UNMATCHINGHASHCHAIN;
				}
				bctoolbox_hmacSha256(messageData->H2, 32, zrtpChannelContext->peerPackets[HELLO_MESSAGE_STORE_ID]->packetString+ZRTP_PACKET_HEADER_LENGTH, zrtpChannelContext->peerPackets[HELLO_MESSAGE_STORE_ID]->messageLength-8, 8, checkMAC);
				if (memcmp(checkMAC, peerHelloMessageData->MAC, 8) != 0) {
					free (messageData);
					return BZRTP_PARSER_ERROR_UNMATCHINGMAC;
				}
				memcpy(messageData->ZID, messageContent, 12);
				messageContent +=12;
				messageData->hashAlgo = cryptoAlgoTypeStringToInt(messageContent, ZRTP_HASH_TYPE);
				messageContent += 4;
				messageData->cipherAlgo = cryptoAlgoTypeStringToInt(messageContent, ZRTP_CIPHERBLOCK_TYPE);
				messageContent += 4;
				messageData->authTagAlgo = cryptoAlgoTypeStringToInt(messageContent, ZRTP_AUTHTAG_TYPE);
				messageContent += 4;
				messageData->keyAgreementAlgo = cryptoAlgoTypeStringToInt(messageContent, ZRTP_KEYAGREEMENT_TYPE);
				messageContent += 4;
				switch(messageData->keyAgreementAlgo) {
					case ZRTP_KEYAGREEMENT_DH2k :
					case ZRTP_KEYAGREEMENT_EC25 :
					case ZRTP_KEYAGREEMENT_DH3k :
					case ZRTP_KEYAGREEMENT_EC38 :
					case ZRTP_KEYAGREEMENT_EC52 :
						variableLength = 32;  
						break;
					case ZRTP_KEYAGREEMENT_Prsh :
						variableLength = 24;  
						break;
					case ZRTP_KEYAGREEMENT_Mult :
						variableLength = 16;  
						break;
					default:
						free(messageData);
						return BZRTP_PARSER_ERROR_INVALIDMESSAGE;
				}
				if (zrtpPacket->messageLength != ZRTP_COMMITMESSAGE_FIXED_LENGTH + variableLength) {
					free(messageData);
					return BZRTP_PARSER_ERROR_INVALIDMESSAGE;
				}
				messageData->sasAlgo = cryptoAlgoTypeStringToInt(messageContent, ZRTP_SAS_TYPE);
				messageContent += 4;
				if ((messageData->keyAgreementAlgo == ZRTP_KEYAGREEMENT_Prsh) || (messageData->keyAgreementAlgo == ZRTP_KEYAGREEMENT_Mult)) {
					memcpy(messageData->nonce, messageContent, 16);
					messageContent +=16;
					if (messageData->keyAgreementAlgo == ZRTP_KEYAGREEMENT_Prsh) {
						memcpy(messageData->keyID, messageContent, 8);
						messageContent +=8;
					}
				} else {  
					memcpy(messageData->hvi, messageContent, 32);
					messageContent +=32;
				}
				memcpy(messageData->MAC, messageContent, 8);
				zrtpPacket->messageData = (void *)messageData;
				zrtpPacket->packetString = (uint8_t *)malloc(inputLength*sizeof(uint8_t));
				memcpy(zrtpPacket->packetString, input, inputLength);  
			}
			break;  
		case MSGTYPE_DHPART1 :
		case MSGTYPE_DHPART2 :
			{
				bzrtpDHPartMessage_t *messageData;
				uint16_t pvLength = computeKeyAgreementPrivateValueLength(zrtpChannelContext->keyAgreementAlgo);
				if (pvLength == 0) {
					return BZRTP_PARSER_ERROR_INVALIDCONTEXT;
				}
				if (zrtpPacket->messageLength != ZRTP_DHPARTMESSAGE_FIXED_LENGTH+pvLength) {
					return BZRTP_PARSER_ERROR_INVALIDMESSAGE;
				}
				messageData = (bzrtpDHPartMessage_t *)malloc(sizeof(bzrtpDHPartMessage_t));
				messageData->pv = (uint8_t *)malloc(pvLength*sizeof(uint8_t));
				memcpy(messageData->H1, messageContent, 32);
				messageContent +=32;
				if ( zrtpChannelContext->role == RESPONDER) {  
					uint8_t checkH2[32];
					uint8_t checkMAC[32];
					bzrtpCommitMessage_t *peerCommitMessageData;
					if (zrtpChannelContext->peerPackets[COMMIT_MESSAGE_STORE_ID] == NULL) {
						free (messageData);
						return BZRTP_PARSER_ERROR_UNEXPECTEDMESSAGE;
					}
					peerCommitMessageData = (bzrtpCommitMessage_t *)zrtpChannelContext->peerPackets[COMMIT_MESSAGE_STORE_ID]->messageData;
					bctoolbox_sha256(messageData->H1, 32, 32, checkH2);
					if (memcmp(checkH2, peerCommitMessageData->H2, 32) != 0) {
						free (messageData);
						return BZRTP_PARSER_ERROR_UNMATCHINGHASHCHAIN;
					}
					bctoolbox_hmacSha256(messageData->H1, 32, zrtpChannelContext->peerPackets[COMMIT_MESSAGE_STORE_ID]->packetString+ZRTP_PACKET_HEADER_LENGTH, zrtpChannelContext->peerPackets[COMMIT_MESSAGE_STORE_ID]->messageLength-8, 8, checkMAC);
					if (memcmp(checkMAC, peerCommitMessageData->MAC, 8) != 0) {
						free (messageData);
						return BZRTP_PARSER_ERROR_UNMATCHINGMAC;
					}
				} else {  
					uint8_t checkH2[32];
					uint8_t checkH3[32];
					uint8_t checkMAC[32];
					bzrtpHelloMessage_t *peerHelloMessageData;
					if (zrtpChannelContext->peerPackets[HELLO_MESSAGE_STORE_ID] == NULL) {
						free (messageData);
						return BZRTP_PARSER_ERROR_UNEXPECTEDMESSAGE;
					}
					peerHelloMessageData = (bzrtpHelloMessage_t *)zrtpChannelContext->peerPackets[HELLO_MESSAGE_STORE_ID]->messageData;
					bctoolbox_sha256(messageData->H1, 32, 32, checkH2);
					bctoolbox_sha256(checkH2, 32, 32, checkH3);
					if (memcmp(checkH3, peerHelloMessageData->H3, 32) != 0) {
						free (messageData);
						return BZRTP_PARSER_ERROR_UNMATCHINGHASHCHAIN;
					}
					bctoolbox_hmacSha256(checkH2, 32, zrtpChannelContext->peerPackets[HELLO_MESSAGE_STORE_ID]->packetString+ZRTP_PACKET_HEADER_LENGTH, zrtpChannelContext->peerPackets[HELLO_MESSAGE_STORE_ID]->messageLength-8, 8, checkMAC);
					if (memcmp(checkMAC, peerHelloMessageData->MAC, 8) != 0) {
						free (messageData);
						return BZRTP_PARSER_ERROR_UNMATCHINGMAC;
					}
				}
				memcpy(messageData->rs1ID, messageContent, 8);
				messageContent +=8;
				memcpy(messageData->rs2ID, messageContent, 8);
				messageContent +=8;
				memcpy(messageData->auxsecretID, messageContent, 8);
				messageContent +=8;
				memcpy(messageData->pbxsecretID, messageContent, 8);
				messageContent +=8;
				memcpy(messageData->pv, messageContent, pvLength);
				messageContent +=pvLength;
				memcpy(messageData->MAC, messageContent, 8);
				zrtpPacket->messageData = (void *)messageData;
				zrtpPacket->packetString = (uint8_t *)malloc(inputLength*sizeof(uint8_t));
				memcpy(zrtpPacket->packetString, input, inputLength);  
			}
			break;  
		case MSGTYPE_CONFIRM1:
		case MSGTYPE_CONFIRM2:
			{
				uint8_t *confirmMessageKey = NULL;
				uint8_t *confirmMessageMacKey = NULL;
				bzrtpConfirmMessage_t *messageData;
				uint16_t cipherTextLength;
				uint8_t computedHmac[8];
				uint8_t *confirmPlainMessageBuffer;
				uint8_t *confirmPlainMessage;
				if (zrtpChannelContext->role == RESPONDER) {  
					if ((zrtpChannelContext->zrtpkeyi == NULL) || (zrtpChannelContext->mackeyi == NULL)) {
						return BZRTP_PARSER_ERROR_INVALIDCONTEXT;
					}
					confirmMessageKey = zrtpChannelContext->zrtpkeyi;
					confirmMessageMacKey = zrtpChannelContext->mackeyi;
				}
				if (zrtpChannelContext->role == INITIATOR) {  
					if ((zrtpChannelContext->zrtpkeyr == NULL) || (zrtpChannelContext->mackeyr == NULL)) {
						return BZRTP_PARSER_ERROR_INVALIDCONTEXT;
					}
					confirmMessageKey = zrtpChannelContext->zrtpkeyr;
					confirmMessageMacKey = zrtpChannelContext->mackeyr;
				}
				messageData = (bzrtpConfirmMessage_t *)malloc(sizeof(bzrtpConfirmMessage_t));
				memcpy(messageData->confirm_mac, messageContent, 8);
				messageContent +=8;
				memcpy(messageData->CFBIV, messageContent, 16);
				messageContent +=16;
				cipherTextLength = zrtpPacket->messageLength - ZRTP_MESSAGE_HEADER_LENGTH - 24;  
				zrtpChannelContext->hmacFunction(confirmMessageMacKey, zrtpChannelContext->hashLength, messageContent, cipherTextLength, 8, computedHmac);
				if (memcmp(computedHmac, messageData->confirm_mac, 8) != 0) {  
					free(messageData);
					return BZRTP_PARSER_ERROR_UNMATCHINGCONFIRMMAC;
				}
				confirmPlainMessageBuffer = (uint8_t *)malloc(cipherTextLength*sizeof(uint8_t));
				zrtpChannelContext->cipherDecryptionFunction(confirmMessageKey, messageData->CFBIV, messageContent, cipherTextLength, confirmPlainMessageBuffer);
				confirmPlainMessage = confirmPlainMessageBuffer;  
				memcpy(messageData->H0, confirmPlainMessage, 32);
				confirmPlainMessage +=33;  
				if (zrtpChannelContext->keyAgreementAlgo == ZRTP_KEYAGREEMENT_Prsh || zrtpChannelContext->keyAgreementAlgo == ZRTP_KEYAGREEMENT_Mult) {
					uint8_t checkH1[32];
					bctoolbox_sha256(messageData->H0, 32, 32, checkH1);
					if ( zrtpChannelContext->role == RESPONDER) {
						uint8_t checkH2[32];
						uint8_t checkMAC[32];
						bzrtpCommitMessage_t *peerCommitMessageData;
						if (zrtpChannelContext->peerPackets[COMMIT_MESSAGE_STORE_ID] == NULL) {
							free (messageData);
							return BZRTP_PARSER_ERROR_UNEXPECTEDMESSAGE;
						}
						peerCommitMessageData = (bzrtpCommitMessage_t *)zrtpChannelContext->peerPackets[COMMIT_MESSAGE_STORE_ID]->messageData;
						bctoolbox_sha256(checkH1, 32, 32, checkH2);
						if (memcmp(checkH2, peerCommitMessageData->H2, 32) != 0) {
							free (messageData);
							return BZRTP_PARSER_ERROR_UNMATCHINGHASHCHAIN;
						}
						bctoolbox_hmacSha256(checkH1, 32, zrtpChannelContext->peerPackets[COMMIT_MESSAGE_STORE_ID]->packetString+ZRTP_PACKET_HEADER_LENGTH, zrtpChannelContext->peerPackets[COMMIT_MESSAGE_STORE_ID]->messageLength-8, 8, checkMAC);
						if (memcmp(checkMAC, peerCommitMessageData->MAC, 8) != 0) {
							free (messageData);
							return BZRTP_PARSER_ERROR_UNMATCHINGMAC;
						}
					} else {  
						uint8_t checkH2[32];
						uint8_t checkH3[32];
						uint8_t checkMAC[32];
						bzrtpHelloMessage_t *peerHelloMessageData;
						if (zrtpChannelContext->peerPackets[HELLO_MESSAGE_STORE_ID] == NULL) {
							free (messageData);
							return BZRTP_PARSER_ERROR_UNEXPECTEDMESSAGE;
						}
						peerHelloMessageData = (bzrtpHelloMessage_t *)zrtpChannelContext->peerPackets[HELLO_MESSAGE_STORE_ID]->messageData;
						bctoolbox_sha256(checkH1, 32, 32, checkH2);
						bctoolbox_sha256(checkH2, 32, 32, checkH3);
						if (memcmp(checkH3, peerHelloMessageData->H3, 32) != 0) {
							free (messageData);
							return BZRTP_PARSER_ERROR_UNMATCHINGHASHCHAIN;
						}
						bctoolbox_hmacSha256(checkH2, 32, zrtpChannelContext->peerPackets[HELLO_MESSAGE_STORE_ID]->packetString+ZRTP_PACKET_HEADER_LENGTH, zrtpChannelContext->peerPackets[HELLO_MESSAGE_STORE_ID]->messageLength-8, 8, checkMAC);
						if (memcmp(checkMAC, peerHelloMessageData->MAC, 8) != 0) {
							free (messageData);
							return BZRTP_PARSER_ERROR_UNMATCHINGMAC;
						}
					}
				} else {  
					uint8_t checkH1[32];
					uint8_t checkMAC[32];
					bzrtpDHPartMessage_t *peerDHPartMessageData;
					if (zrtpChannelContext->peerPackets[DHPART_MESSAGE_STORE_ID] == NULL) {
						free (messageData);
						return BZRTP_PARSER_ERROR_UNEXPECTEDMESSAGE;
					}
					peerDHPartMessageData = (bzrtpDHPartMessage_t *)zrtpChannelContext->peerPackets[DHPART_MESSAGE_STORE_ID]->messageData;
					bctoolbox_sha256(messageData->H0, 32, 32, checkH1);
					if (memcmp(checkH1, peerDHPartMessageData->H1, 32) != 0) {
						free (messageData);
						return BZRTP_PARSER_ERROR_UNMATCHINGHASHCHAIN;
					}
					bctoolbox_hmacSha256(messageData->H0, 32, zrtpChannelContext->peerPackets[DHPART_MESSAGE_STORE_ID]->packetString+ZRTP_PACKET_HEADER_LENGTH, zrtpChannelContext->peerPackets[DHPART_MESSAGE_STORE_ID]->messageLength-8, 8, checkMAC);
					if (memcmp(checkMAC, peerDHPartMessageData->MAC, 8) != 0) {
						free (messageData);
						return BZRTP_PARSER_ERROR_UNMATCHINGMAC;
					}
				}
				messageData->sig_len = ((uint16_t)(confirmPlainMessage[0]&0x01))<<8 | (((uint16_t)confirmPlainMessage[1])&0x00FF);
				confirmPlainMessage += 2;
				messageData->E = ((*confirmPlainMessage)&0x08)>>3;
				messageData->V = ((*confirmPlainMessage)&0x04)>>2;
				messageData->A = ((*confirmPlainMessage)&0x02)>>1;
				messageData->D = (*confirmPlainMessage)&0x01;
				confirmPlainMessage += 1;
				messageData->cacheExpirationInterval = (((uint32_t)confirmPlainMessage[0])<<24) | (((uint32_t)confirmPlainMessage[1])<<16) | (((uint32_t)confirmPlainMessage[2])<<8) | ((uint32_t)confirmPlainMessage[3]);
				confirmPlainMessage += 4;
				if (messageData->sig_len>0) {
					memcpy(messageData->signatureBlockType, confirmPlainMessage, 4);
					confirmPlainMessage += 4;
					messageData->signatureBlock = (uint8_t *)malloc(4*(messageData->sig_len-1)*sizeof(uint8_t));
					memcpy(messageData->signatureBlock, confirmPlainMessage, 4*(messageData->sig_len-1));
				} else {
					messageData->signatureBlock  = NULL;
				}
				free(confirmPlainMessageBuffer);
				zrtpPacket->packetString = (uint8_t *)malloc(inputLength*sizeof(uint8_t));
				memcpy(zrtpPacket->packetString, input, inputLength);  
				zrtpPacket->messageData = (void *)messageData;
			}
			break;  
		case MSGTYPE_CONF2ACK:
			break;  
		case MSGTYPE_PING:
			{
				bzrtpPingMessage_t *messageData;
				messageData = (bzrtpPingMessage_t *)malloc(sizeof(bzrtpPingMessage_t));
				memcpy(messageData->version, messageContent, 4);
				messageContent +=4;
				memcpy(messageData->endpointHash, messageContent, 8);
				zrtpPacket->messageData = (void *)messageData;
			}
			break;  
	}
	return 0;
}