PJ_DEF(pj_status_t) pjsip_ua_register_dlg( pjsip_user_agent *ua,
					   pjsip_dialog *dlg )
{
    PJ_ASSERT_RETURN(ua && dlg, PJ_EINVAL);
    PJ_ASSERT_RETURN(dlg->local.info && dlg->local.info->tag.slen &&
		     dlg->local.tag_hval != 0, PJ_EBUG);
    pj_mutex_lock(mod_ua.mutex);
    if (dlg->role == PJSIP_ROLE_UAC) {
	struct dlg_set *dlg_set;
	dlg_set = (struct dlg_set*)
		  pj_hash_get_lower( mod_ua.dlg_table,
                                     dlg->local.info->tag.ptr, 
			             (unsigned)dlg->local.info->tag.slen,
			             &dlg->local.tag_hval);
	if (dlg_set) {
	    pj_assert(dlg_set->dlg_list.next != (void*)&dlg_set->dlg_list);
	    pj_list_push_back(&dlg_set->dlg_list, dlg);
	    dlg->dlg_set = dlg_set;
	} else {
	    dlg_set = alloc_dlgset_node();
	    pj_list_init(&dlg_set->dlg_list);
	    pj_list_push_back(&dlg_set->dlg_list, dlg);
	    dlg->dlg_set = dlg_set;
	    pj_hash_set_np_lower(mod_ua.dlg_table, 
			         dlg->local.info->tag.ptr,
                                 (unsigned)dlg->local.info->tag.slen,
			         dlg->local.tag_hval, dlg_set->ht_entry,
                                 dlg_set);
	}
    } else {
	struct dlg_set *dlg_set;
	dlg_set = alloc_dlgset_node();
	pj_list_init(&dlg_set->dlg_list);
	pj_list_push_back(&dlg_set->dlg_list, dlg);
	dlg->dlg_set = dlg_set;
	pj_hash_set_np_lower(mod_ua.dlg_table, 
		             dlg->local.info->tag.ptr,
                             (unsigned)dlg->local.info->tag.slen,
		             dlg->local.tag_hval, dlg_set->ht_entry, dlg_set);
    }
    pj_mutex_unlock(mod_ua.mutex);
    return PJ_SUCCESS;
}