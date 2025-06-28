mmsClient_handleFileOpenRequest(
    MmsConnection connection,
    uint8_t* buffer, int bufPos, int maxBufPos,
    uint32_t invokeId, ByteBuffer* response)
{
    char filename[256];
    bool hasFileName = false;
    uint32_t filePosition = 0;
    while (bufPos < maxBufPos) {
        uint8_t tag = buffer[bufPos++];
        int length;
        bufPos = BerDecoder_decodeLength(buffer, &length, bufPos, maxBufPos);
        if (bufPos < 0) goto exit_reject_invalid_pdu;
        switch(tag) {
        case 0xa0:  
            if (!mmsMsg_parseFileName(filename, buffer, &bufPos, bufPos + length, invokeId, response))
                return;
            hasFileName = true;
            break;
        case 0x81:  
            filePosition = BerDecoder_decodeUint32(buffer, length, bufPos);
            bufPos += length;
            break;
        case 0x00:  
            break;
        default:  
            bufPos += length;
            goto exit_reject_invalid_pdu;
        }
    }
    if (hasFileName) {
        MmsFileReadStateMachine* frsm = getFreeFrsm(connection);
        if (frsm != NULL) {
            MmsOutstandingCall obtainFileCall = mmsClient_getMatchingObtainFileRequest(connection, filename);
            if (obtainFileCall) {
                if (DEBUG_MMS_CLIENT)
                    printf("MMS_CLIENT: file open is matching obtain file request for file %s\n", filename);
                obtainFileCall->timeout = Hal_getTimeInMs() + connection->requestTimeout;
            }
            FileHandle fileHandle = mmsMsg_openFile(MmsConnection_getFilestoreBasepath(connection), filename, false);
            if (fileHandle != NULL) {
                frsm->fileHandle = fileHandle;
                frsm->readPosition = filePosition;
                frsm->frsmId = getNextFrsmId(connection);
                frsm->obtainRequest = obtainFileCall;
                mmsMsg_createFileOpenResponse(MmsConnection_getFilestoreBasepath(connection),
                        invokeId, response, filename, frsm);
            }
            else
                mmsMsg_createServiceErrorPdu(invokeId, response, MMS_ERROR_FILE_FILE_NON_EXISTENT);
        }
        else
            mmsMsg_createServiceErrorPdu(invokeId, response, MMS_ERROR_RESOURCE_OTHER);
    }
    else
        goto exit_invalid_parameter;
    return;
exit_invalid_parameter:
    mmsMsg_createMmsRejectPdu(&invokeId, MMS_ERROR_REJECT_REQUEST_INVALID_ARGUMENT, response);
    return;
exit_reject_invalid_pdu:
    mmsMsg_createMmsRejectPdu(&invokeId, MMS_ERROR_REJECT_INVALID_PDU, response);
}