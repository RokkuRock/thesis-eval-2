PJ_DEF(pj_status_t) pjsip_ua_unregister_dlg( pjsip_user_agent *ua,
					     pjsip_dialog *dlg )
{
    struct dlg_set *dlg_set;
    pjsip_dialog *d;
    PJ_ASSERT_RETURN(ua && dlg, PJ_EINVAL);
    PJ_ASSERT_RETURN(dlg->dlg_set, PJ_EINVALIDOP);
    pj_mutex_lock(mod_ua.mutex);
    dlg_set = (struct dlg_set*) dlg->dlg_set;
    d = dlg_set->dlg_list.next;
    while (d != (pjsip_dialog*)&dlg_set->dlg_list && d != dlg) {
	d = d->next;
    }
    if (d != dlg) {
	pj_assert(!"Dialog is not registered!");
	pj_mutex_unlock(mod_ua.mutex);
	return PJ_EINVALIDOP;
    }
    pj_list_erase(dlg);
    if (pj_list_empty(&dlg_set->dlg_list)) {
	pj_hash_set_lower(NULL, mod_ua.dlg_table, dlg->local.info->tag.ptr,
		          (unsigned)dlg->local.info->tag.slen, 
			  dlg->local.tag_hval, NULL);
	pj_list_push_back(&mod_ua.free_dlgset_nodes, dlg_set);
    }
    pj_mutex_unlock(mod_ua.mutex);
    return PJ_SUCCESS;
}