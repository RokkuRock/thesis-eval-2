PJ_DEF(pj_status_t) pjmedia_sdp_neg_modify_local_offer2(
                                    pj_pool_t *pool,
				    pjmedia_sdp_neg *neg,
                                    unsigned flags,
				    const pjmedia_sdp_session *local)
{
    pjmedia_sdp_session *new_offer;
    pjmedia_sdp_session *old_offer;
    char media_used[PJMEDIA_MAX_SDP_MEDIA];
    unsigned oi;  
    pj_status_t status;
    PJ_ASSERT_RETURN(pool && neg && local, PJ_EINVAL);
    PJ_ASSERT_RETURN(neg->state == PJMEDIA_SDP_NEG_STATE_DONE, 
		     PJMEDIA_SDPNEG_EINSTATE);
    status = pjmedia_sdp_validate(local);
    if (status != PJ_SUCCESS)
	return status;
    neg->state = PJMEDIA_SDP_NEG_STATE_LOCAL_OFFER;
    pj_bzero(media_used, sizeof(media_used));
    old_offer = neg->active_local_sdp;
    new_offer = pjmedia_sdp_session_clone(pool, local);
    pj_strdup(pool, &new_offer->origin.user, &old_offer->origin.user);
    new_offer->origin.id = old_offer->origin.id;
    pj_strdup(pool, &new_offer->origin.net_type, &old_offer->origin.net_type);
    pj_strdup(pool, &new_offer->origin.addr_type,&old_offer->origin.addr_type);
    pj_strdup(pool, &new_offer->origin.addr, &old_offer->origin.addr);
    if ((flags & PJMEDIA_SDP_NEG_ALLOW_MEDIA_CHANGE) == 0) {
        for (oi = 0; oi < old_offer->media_count; ++oi) {
	    pjmedia_sdp_media *om;
	    pjmedia_sdp_media *nm;
	    unsigned ni;  
	    pj_bool_t found = PJ_FALSE;
	    om = old_offer->media[oi];
	    for (ni = oi; ni < new_offer->media_count; ++ni) {
	        nm = new_offer->media[ni];
	        if (pj_strcmp(&nm->desc.media, &om->desc.media) == 0) {
		    if (ni != oi) {
		        pj_array_insert(
                            new_offer->media,		  
			    sizeof(new_offer->media[0]),  
			    ni,				  
		            oi,				  
			    &nm);			  
		    }
		    found = PJ_TRUE;
		    break;
	        }
	    }
	    if (!found) {
	        pjmedia_sdp_media *m;
	        m = sdp_media_clone_deactivate(pool, om, om, local);
	        pj_array_insert(new_offer->media, sizeof(new_offer->media[0]),
			        new_offer->media_count++, oi, &m);
	    }
        }
    } else {
        for (oi = new_offer->media_count; oi < old_offer->media_count; ++oi) {
            pjmedia_sdp_media *m;
	    m = sdp_media_clone_deactivate(pool, old_offer->media[oi],
                                           old_offer->media[oi], local);
	    pj_array_insert(new_offer->media, sizeof(new_offer->media[0]),
	                    new_offer->media_count++, oi, &m);
        }
    }
#if PJMEDIA_SDP_NEG_COMPARE_BEFORE_INC_VERSION
    new_offer->origin.version = old_offer->origin.version;
    if (pjmedia_sdp_session_cmp(new_offer, neg->initial_sdp, 0) != PJ_SUCCESS)
    {
	++new_offer->origin.version;
    }    
#else
    new_offer->origin.version = old_offer->origin.version + 1;
#endif
    neg->initial_sdp_tmp = neg->initial_sdp;
    neg->initial_sdp = new_offer;
    neg->neg_local_sdp = pjmedia_sdp_session_clone(pool, new_offer);
    return PJ_SUCCESS;
}