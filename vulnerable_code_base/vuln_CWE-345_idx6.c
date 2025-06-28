int ParseCertRelative(DecodedCert* cert, int type, int verify, void* cm)
{
    int    ret = 0;
    int    checkPathLen = 0;
    int    decrementMaxPathLen = 0;
    word32 confirmOID = 0;
#if defined(WOLFSSL_RENESAS_TSIP)
    int    idx = 0;
#endif
    byte*  tsip_encRsaKeyIdx;
#ifdef WOLFSSL_CERT_REQ
    int    len = 0;
#endif
    if (cert == NULL) {
        return BAD_FUNC_ARG;
    }
#ifdef WOLFSSL_CERT_REQ
    if (type == CERTREQ_TYPE)
        cert->isCSR = 1;
#endif
    if (cert->sigCtx.state == SIG_STATE_BEGIN) {
        cert->badDate = 0;
        cert->criticalExt = 0;
        if ((ret = DecodeToKey(cert, verify)) < 0) {
            if (ret == ASN_BEFORE_DATE_E || ret == ASN_AFTER_DATE_E)
                cert->badDate = ret;
            else
                return ret;
        }
        WOLFSSL_MSG("Parsed Past Key");
#ifdef WOLFSSL_CERT_REQ
        if (cert->isCSR) {
            if (GetASNHeader_ex(cert->source,
                    ASN_CONTEXT_SPECIFIC | ASN_CONSTRUCTED, &cert->srcIdx,
                    &len, cert->maxIdx, 1) < 0) {
                WOLFSSL_MSG("GetASNHeader_ex error");
                return ASN_PARSE_E;
            }
            if (len) {
                word32 attrMaxIdx = cert->srcIdx + len;
                word32 oid;
                byte   tag;
                if (attrMaxIdx > cert->maxIdx) {
                    WOLFSSL_MSG("Attribute length greater than CSR length");
                    return ASN_PARSE_E;
                }
                while (cert->srcIdx < attrMaxIdx) {
                    if (GetSequence(cert->source, &cert->srcIdx, &len,
                            attrMaxIdx) < 0) {
                        WOLFSSL_MSG("attr GetSequence error");
                        return ASN_PARSE_E;
                    }
                    if (GetObjectId(cert->source, &cert->srcIdx, &oid,
                            oidCsrAttrType, attrMaxIdx) < 0) {
                        WOLFSSL_MSG("attr GetObjectId error");
                        return ASN_PARSE_E;
                    }
                    if (GetSet(cert->source, &cert->srcIdx, &len,
                            attrMaxIdx) < 0) {
                        WOLFSSL_MSG("attr GetSet error");
                        return ASN_PARSE_E;
                    }
                    switch (oid) {
                    case CHALLENGE_PASSWORD_OID:
                        if (GetHeader(cert->source, &tag,
                                &cert->srcIdx, &len, attrMaxIdx, 1) < 0) {
                            WOLFSSL_MSG("attr GetHeader error");
                            return ASN_PARSE_E;
                        }
                        if (tag != ASN_PRINTABLE_STRING && tag != ASN_UTF8STRING &&
                                tag != ASN_IA5_STRING) {
                            WOLFSSL_MSG("Unsupported attribute value format");
                            return ASN_PARSE_E;
                        }
                        cert->cPwd = (char*)cert->source + cert->srcIdx;
                        cert->cPwdLen = len;
                        cert->srcIdx += len;
                        break;
                    case SERIAL_NUMBER_OID:
                        if (GetHeader(cert->source, &tag,
                                &cert->srcIdx, &len, attrMaxIdx, 1) < 0) {
                            WOLFSSL_MSG("attr GetHeader error");
                            return ASN_PARSE_E;
                        }
                        if (tag != ASN_PRINTABLE_STRING && tag != ASN_UTF8STRING &&
                                tag != ASN_IA5_STRING) {
                            WOLFSSL_MSG("Unsupported attribute value format");
                            return ASN_PARSE_E;
                        }
                        cert->sNum = (char*)cert->source + cert->srcIdx;
                        cert->sNumLen = len;
                        cert->srcIdx += len;
                        if (cert->sNumLen <= EXTERNAL_SERIAL_SIZE) {
                            XMEMCPY(cert->serial, cert->sNum, cert->sNumLen);
                            cert->serialSz = cert->sNumLen;
                        }
                        break;
                    case EXTENSION_REQUEST_OID:
                        cert->extensions    = &cert->source[cert->srcIdx];
                        cert->extensionsSz  = len;
                        cert->extensionsIdx = cert->srcIdx;    
                        if ((ret = DecodeCertExtensions(cert)) < 0) {
                            if (ret == ASN_CRIT_EXT_E)
                                cert->criticalExt = ret;
                            else
                                return ret;
                        }
                        cert->srcIdx += len;
                        break;
                    default:
                        WOLFSSL_MSG("Unsupported attribute type");
                        return ASN_PARSE_E;
                    }
                }
            }
        }
#endif
        if (cert->srcIdx < cert->sigIndex) {
        #ifndef ALLOW_V1_EXTENSIONS
            if (cert->version < 2) {
                WOLFSSL_MSG("\tv1 and v2 certs not allowed extensions");
                return ASN_VERSION_E;
            }
        #endif
            cert->extensions    = &cert->source[cert->srcIdx];
            cert->extensionsSz  = cert->sigIndex - cert->srcIdx;
            cert->extensionsIdx = cert->srcIdx;    
            if ((ret = DecodeCertExtensions(cert)) < 0) {
                if (ret == ASN_CRIT_EXT_E)
                    cert->criticalExt = ret;
                else
                    return ret;
            }
        #ifdef HAVE_OCSP
            if (cert->ocspNoCheckSet && verify == VERIFY_OCSP)
                verify = NO_VERIFY;
        #endif
            cert->srcIdx = cert->sigIndex;
        }
        if ((ret = GetAlgoId(cert->source, &cert->srcIdx,
#ifdef WOLFSSL_CERT_REQ
                !cert->isCSR ? &confirmOID : &cert->signatureOID,
#else
                &confirmOID,
#endif
                oidSigType, cert->maxIdx)) < 0)
            return ret;
        if ((ret = GetSignature(cert)) < 0)
            return ret;
        if (confirmOID != cert->signatureOID
#ifdef WOLFSSL_CERT_REQ
                && !cert->isCSR
#endif
                )
            return ASN_SIG_OID_E;
    #ifndef NO_SKID
        if (cert->extSubjKeyIdSet == 0 && cert->publicKey != NULL &&
                                                         cert->pubKeySize > 0) {
            ret = CalcHashId(cert->publicKey, cert->pubKeySize,
                                                            cert->extSubjKeyId);
            if (ret != 0)
                return ret;
        }
    #endif  
        if (!cert->selfSigned || (verify != NO_VERIFY && type != CA_TYPE &&
                                                   type != TRUSTED_PEER_TYPE)) {
            cert->ca = NULL;
    #ifndef NO_SKID
            if (cert->extAuthKeyIdSet) {
                cert->ca = GetCA(cm, cert->extAuthKeyId);
            }
            if (cert->ca == NULL && cert->extSubjKeyIdSet
                                 && verify != VERIFY_OCSP) {
                cert->ca = GetCA(cm, cert->extSubjKeyId);
            }
            if (cert->ca != NULL && XMEMCMP(cert->issuerHash,
                                  cert->ca->subjectNameHash, KEYID_SIZE) != 0) {
                cert->ca = NULL;
            }
            if (cert->ca == NULL) {
                cert->ca = GetCAByName(cm, cert->issuerHash);
                if (cert->ca && cert->extAuthKeyIdSet) {
                    WOLFSSL_MSG("CA SKID doesn't match AKID");
                    cert->ca = NULL;
                }
            }
        #ifdef WOLFSSL_NO_TRUSTED_CERTS_VERIFY
            if (cert->ca == NULL && verify == VERIFY_OCSP) {
                cert->ca = GetCABySubjectAndPubKey(cert, cm);
                if (cert->ca) {
                    ret = 0;  
                    goto exit_pcr;
                }
            }
        #endif  
    #else
            cert->ca = GetCA(cm, cert->issuerHash);
    #endif  
            if (cert->ca) {
                WOLFSSL_MSG("CA found");
            }
        }
        if (cert->selfSigned) {
            cert->maxPathLen = WOLFSSL_MAX_PATH_LEN;
        } else {
            if (cert->ca && cert->pathLengthSet) {
                cert->maxPathLen = cert->pathLength;
                if (cert->isCA) {
                    WOLFSSL_MSG("\tCA boolean set");
                    if (cert->extKeyUsageSet) {
                         WOLFSSL_MSG("\tExtension Key Usage Set");
                         if ((cert->extKeyUsage & KEYUSE_KEY_CERT_SIGN) != 0) {
                            checkPathLen = 1;
                         } else {
                            decrementMaxPathLen = 1;
                         }
                    } else {
                        checkPathLen = 1;
                    }  
                }  
                if (checkPathLen && cert->pathLengthSet) {
                    if (cert->pathLength < cert->ca->maxPathLen) {
                        WOLFSSL_MSG("\tmaxPathLen status: set to pathLength");
                        cert->maxPathLen = cert->pathLength;
                    } else {
                        decrementMaxPathLen = 1;
                    }
                }
                if (decrementMaxPathLen && cert->ca->maxPathLen > 0) {
                    WOLFSSL_MSG("\tmaxPathLen status: reduce by 1");
                    cert->maxPathLen = cert->ca->maxPathLen - 1;
                    if (verify != NO_VERIFY && type != CA_TYPE &&
                                                    type != TRUSTED_PEER_TYPE) {
                        WOLFSSL_MSG("\tmaxPathLen status: OK");
                    }
                } else if (decrementMaxPathLen && cert->ca->maxPathLen == 0) {
                    cert->maxPathLen = 0;
                    if (verify != NO_VERIFY && type != CA_TYPE &&
                                                    type != TRUSTED_PEER_TYPE) {
                        WOLFSSL_MSG("\tNon-entity cert, maxPathLen is 0");
                        WOLFSSL_MSG("\tmaxPathLen status: ERROR");
                        return ASN_PATHLEN_INV_E;
                    }
                }
            } else if (cert->ca && cert->isCA) {
                if (cert->ca->maxPathLen > 0) {
                    cert->maxPathLen = cert->ca->maxPathLen - 1;
                } else {
                    cert->maxPathLen = 0;
                    if (verify != NO_VERIFY && type != CA_TYPE &&
                                                    type != TRUSTED_PEER_TYPE) {
                        WOLFSSL_MSG("\tNon-entity cert, maxPathLen is 0");
                        WOLFSSL_MSG("\tmaxPathLen status: ERROR");
                        return ASN_PATHLEN_INV_E;
                    }
                }
            }
        }
        #ifdef HAVE_OCSP
        if (verify != NO_VERIFY && type != CA_TYPE &&
                                                type != TRUSTED_PEER_TYPE) {
            if (cert->ca) {
                XMEMCPY(cert->issuerKeyHash, cert->ca->subjectKeyHash,
                                                                KEYID_SIZE);
            }
        }
        #endif  
    }
#if defined(WOLFSSL_RENESAS_TSIP)
    if (cert->keyOID == RSAk) {
        if ((ret = RsaPublicKeyDecodeRawIndex(cert->publicKey, (word32*)&idx,
                                   cert->pubKeySize,
                                   &cert->sigCtx.pubkey_n_start,
                                   &cert->sigCtx.pubkey_n_len,
                                   &cert->sigCtx.pubkey_e_start,
                                   &cert->sigCtx.pubkey_e_len)) != 0) {
            WOLFSSL_MSG("Decoding index from cert failed.");
            return ret;
        }
        cert->sigCtx.certBegin = cert->certBegin;
    }
    if (cert->ca && tsip_checkCA(cert->ca->cm_idx) != 0 &&
        cert->sigCtx.pubkey_n_len == 256) {
        if (!cert->tsip_encRsaKeyIdx)
            cert->tsip_encRsaKeyIdx =
                            (byte*)XMALLOC(TSIP_TLS_ENCPUBKEY_SZ_BY_CERTVRFY,
                             cert->heap, DYNAMIC_TYPE_RSA);
        if (cert->tsip_encRsaKeyIdx == NULL)
                return MEMORY_E;
    } else {
        if (cert->ca) {
            if (tsip_checkCA(cert->ca->cm_idx) == 0)
                WOLFSSL_MSG("TSIP isn't usable because the ca isn't verified "
                            "by TSIP.");
            else if (cert->sigCtx.pubkey_n_len != 256)
                WOLFSSL_MSG("TSIP isn't usable because the ca isn't signed by "
                            "RSA 2048.");
            else
                WOLFSSL_MSG("TSIP isn't usable");
        }
        cert->tsip_encRsaKeyIdx = NULL;
    }
    tsip_encRsaKeyIdx = cert->tsip_encRsaKeyIdx;
#else
    tsip_encRsaKeyIdx = NULL;
#endif
    if (verify != NO_VERIFY && type != CA_TYPE && type != TRUSTED_PEER_TYPE) {
        if (cert->ca) {
            if (verify == VERIFY || verify == VERIFY_OCSP ||
                                                 verify == VERIFY_SKIP_DATE) {
                if ((ret = ConfirmSignature(&cert->sigCtx,
                        cert->source + cert->certBegin,
                        cert->sigIndex - cert->certBegin,
                        cert->ca->publicKey, cert->ca->pubKeySize,
                        cert->ca->keyOID, cert->signature,
                        cert->sigLength, cert->signatureOID,
                        tsip_encRsaKeyIdx)) != 0) {
                    if (ret != WC_PENDING_E) {
                        WOLFSSL_MSG("Confirm signature failed");
                    }
                    return ret;
                }
            }
        #ifndef IGNORE_NAME_CONSTRAINTS
            if (verify == VERIFY || verify == VERIFY_OCSP ||
                        verify == VERIFY_NAME || verify == VERIFY_SKIP_DATE) {
                if (!ConfirmNameConstraints(cert->ca, cert)) {
                    WOLFSSL_MSG("Confirm name constraint failed");
                    return ASN_NAME_INVALID_E;
                }
            }
        #endif  
        }
        else {
            WOLFSSL_MSG("No CA signer to verify with");
            return ASN_NO_SIGNER_E;
        }
    }
#if defined(WOLFSSL_NO_TRUSTED_CERTS_VERIFY) && !defined(NO_SKID)
exit_pcr:
#endif
    if (cert->badDate != 0) {
        if (verify != VERIFY_SKIP_DATE) {
            return cert->badDate;
        }
        WOLFSSL_MSG("Date error: Verify option is skipping");
    }
    if (cert->criticalExt != 0)
        return cert->criticalExt;
    return ret;
}