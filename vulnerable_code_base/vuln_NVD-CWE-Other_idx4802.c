kadm5_create_principal_3(void *server_handle,
                         kadm5_principal_ent_t entry, long mask,
                         int n_ks_tuple, krb5_key_salt_tuple *ks_tuple,
                         char *password)
{
    krb5_db_entry               *kdb;
    osa_princ_ent_rec           adb;
    kadm5_policy_ent_rec        polent;
    krb5_boolean                have_polent = FALSE;
    krb5_int32                  now;
    krb5_tl_data                *tl_data_tail;
    unsigned int                ret;
    kadm5_server_handle_t handle = server_handle;
    krb5_keyblock               *act_mkey;
    krb5_kvno                   act_kvno;
    int                         new_n_ks_tuple = 0;
    krb5_key_salt_tuple         *new_ks_tuple = NULL;
    CHECK_HANDLE(server_handle);
    krb5_clear_error_message(handle->context);
    check_1_6_dummy(entry, mask, n_ks_tuple, ks_tuple, &password);
    if(!(mask & KADM5_PRINCIPAL) || (mask & KADM5_MOD_NAME) ||
       (mask & KADM5_MOD_TIME) || (mask & KADM5_LAST_PWD_CHANGE) ||
       (mask & KADM5_MKVNO) || (mask & KADM5_AUX_ATTRIBUTES) ||
       (mask & KADM5_LAST_SUCCESS) || (mask & KADM5_LAST_FAILED) ||
       (mask & KADM5_FAIL_AUTH_COUNT))
        return KADM5_BAD_MASK;
    if ((mask & KADM5_KEY_DATA) && entry->n_key_data != 0)
        return KADM5_BAD_MASK;
    if((mask & KADM5_POLICY) && (mask & KADM5_POLICY_CLR))
        return KADM5_BAD_MASK;
    if((mask & ~ALL_PRINC_MASK))
        return KADM5_BAD_MASK;
    if (entry == NULL)
        return EINVAL;
    ret = kdb_get_entry(handle, entry->principal, &kdb, &adb);
    switch(ret) {
    case KADM5_UNK_PRINC:
        break;
    case 0:
        kdb_free_entry(handle, kdb, &adb);
        return KADM5_DUP;
    default:
        return ret;
    }
    kdb = krb5_db_alloc(handle->context, NULL, sizeof(*kdb));
    if (kdb == NULL)
        return ENOMEM;
    memset(kdb, 0, sizeof(*kdb));
    memset(&adb, 0, sizeof(osa_princ_ent_rec));
    if ((mask & KADM5_POLICY)) {
        ret = get_policy(handle, entry->policy, &polent, &have_polent);
        if (ret)
            goto cleanup;
    }
    if (password) {
        ret = passwd_check(handle, password, have_polent ? &polent : NULL,
                           entry->principal);
        if (ret)
            goto cleanup;
    }
    if ((ret = krb5_timeofday(handle->context, &now)))
        goto cleanup;
    kdb->magic = KRB5_KDB_MAGIC_NUMBER;
    kdb->len = KRB5_KDB_V1_BASE_LENGTH;  
    if ((mask & KADM5_ATTRIBUTES))
        kdb->attributes = entry->attributes;
    else
        kdb->attributes = handle->params.flags;
    if ((mask & KADM5_MAX_LIFE))
        kdb->max_life = entry->max_life;
    else
        kdb->max_life = handle->params.max_life;
    if (mask & KADM5_MAX_RLIFE)
        kdb->max_renewable_life = entry->max_renewable_life;
    else
        kdb->max_renewable_life = handle->params.max_rlife;
    if ((mask & KADM5_PRINC_EXPIRE_TIME))
        kdb->expiration = entry->princ_expire_time;
    else
        kdb->expiration = handle->params.expiration;
    kdb->pw_expiration = 0;
    if (have_polent) {
        if(polent.pw_max_life)
            kdb->pw_expiration = now + polent.pw_max_life;
        else
            kdb->pw_expiration = 0;
    }
    if ((mask & KADM5_PW_EXPIRATION))
        kdb->pw_expiration = entry->pw_expiration;
    kdb->last_success = 0;
    kdb->last_failed = 0;
    kdb->fail_auth_count = 0;
    if ((ret = kadm5_copy_principal(handle->context,
                                    entry->principal, &(kdb->princ))))
        goto cleanup;
    if ((ret = krb5_dbe_update_last_pwd_change(handle->context, kdb, now)))
        goto cleanup;
    if (mask & KADM5_TL_DATA) {
        for (tl_data_tail = entry->tl_data; tl_data_tail;
             tl_data_tail = tl_data_tail->tl_data_next)
        {
            ret = krb5_dbe_update_tl_data(handle->context, kdb, tl_data_tail);
            if( ret )
                goto cleanup;
        }
    }
    ret = apply_keysalt_policy(handle, entry->policy, n_ks_tuple, ks_tuple,
                               &new_n_ks_tuple, &new_ks_tuple);
    if (ret)
        goto cleanup;
    ret = kdb_get_active_mkey(handle, &act_kvno, &act_mkey);
    if (ret)
        goto cleanup;
    if (mask & KADM5_KEY_DATA) {
        assert(entry->n_key_data == 0);
    } else if (password) {
        ret = krb5_dbe_cpw(handle->context, act_mkey, new_ks_tuple,
                           new_n_ks_tuple, password,
                           (mask & KADM5_KVNO)?entry->kvno:1,
                           FALSE, kdb);
    } else {
        ret = krb5_dbe_crk(handle->context, &master_keyblock,
                           new_ks_tuple, new_n_ks_tuple, FALSE, kdb);
    }
    if (ret)
        goto cleanup;
    ret = krb5_dbe_update_mkvno(handle->context, kdb, act_kvno);
    if (ret)
        goto cleanup;
    ret = k5_kadm5_hook_create(handle->context, handle->hook_handles,
                               KADM5_HOOK_STAGE_PRECOMMIT, entry, mask,
                               new_n_ks_tuple, new_ks_tuple, password);
    if (ret)
        goto cleanup;
    adb.admin_history_kvno = INITIAL_HIST_KVNO;
    if (mask & KADM5_POLICY) {
        adb.aux_attributes = KADM5_POLICY;
        adb.policy = entry->policy;
    }
    kdb->mask = mask | KADM5_KEY_DATA | KADM5_PRINCIPAL ;
    ret = kdb_put_entry(handle, kdb, &adb);
    (void) k5_kadm5_hook_create(handle->context, handle->hook_handles,
                                KADM5_HOOK_STAGE_POSTCOMMIT, entry, mask,
                                new_n_ks_tuple, new_ks_tuple, password);
cleanup:
    free(new_ks_tuple);
    krb5_db_free_principal(handle->context, kdb);
    if (have_polent)
        (void) kadm5_free_policy_ent(handle->lhandle, &polent);
    return ret;
}