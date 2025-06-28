krb5_ldap_put_principal(krb5_context context, krb5_db_entry *entry,
                        char **db_args)
{
    int                         l=0, kerberos_principal_object_type=0;
    unsigned int                ntrees=0, tre=0;
    krb5_error_code             st=0, tempst=0;
    LDAP                        *ld=NULL;
    LDAPMessage                 *result=NULL, *ent=NULL;
    char                        **subtreelist = NULL;
    char                        *user=NULL, *subtree=NULL, *principal_dn=NULL;
    char                        **values=NULL, *strval[10]={NULL}, errbuf[1024];
    char                        *filtuser=NULL;
    struct berval               **bersecretkey=NULL;
    LDAPMod                     **mods=NULL;
    krb5_boolean                create_standalone_prinicipal=FALSE;
    krb5_boolean                krb_identity_exists=FALSE, establish_links=FALSE;
    char                        *standalone_principal_dn=NULL;
    krb5_tl_data                *tl_data=NULL;
    krb5_key_data               **keys=NULL;
    kdb5_dal_handle             *dal_handle=NULL;
    krb5_ldap_context           *ldap_context=NULL;
    krb5_ldap_server_handle     *ldap_server_handle=NULL;
    osa_princ_ent_rec           princ_ent = {0};
    xargs_t                     xargs = {0};
    char                        *polname = NULL;
    OPERATION optype;
    krb5_boolean                found_entry = FALSE;
    krb5_clear_error_message(context);
    SETUP_CONTEXT();
    if (ldap_context->lrparams == NULL || ldap_context->container_dn == NULL)
        return EINVAL;
    GET_HANDLE();
    if (!is_principal_in_realm(ldap_context, entry->princ)) {
        st = EINVAL;
        k5_setmsg(context, st,
                  _("Principal does not belong to the default realm"));
        goto cleanup;
    }
    if (((st=krb5_unparse_name(context, entry->princ, &user)) != 0) ||
        ((st=krb5_ldap_unparse_principal_name(user)) != 0))
        goto cleanup;
    filtuser = ldap_filter_correct(user);
    if (filtuser == NULL) {
        st = ENOMEM;
        goto cleanup;
    }
    if (entry->mask & KADM5_PRINCIPAL)
        optype = ADD_PRINCIPAL;
    else
        optype = MODIFY_PRINCIPAL;
    if (((st=krb5_get_princ_type(context, entry, &kerberos_principal_object_type)) != 0) ||
        ((st=krb5_get_userdn(context, entry, &principal_dn)) != 0))
        goto cleanup;
    if ((st=process_db_args(context, db_args, &xargs, optype)) != 0)
        goto cleanup;
    if (entry->mask & KADM5_LOAD) {
        unsigned int     tree = 0;
        int              numlentries = 0;
        char             *filter = NULL;
        if (asprintf(&filter, FILTER"%s))", filtuser) < 0) {
            filter = NULL;
            st = ENOMEM;
            goto cleanup;
        }
        if ((st = krb5_get_subtree_info(ldap_context, &subtreelist, &ntrees)) != 0)
            goto cleanup;
        found_entry = FALSE;
        for (tree = 0; found_entry == FALSE && tree < ntrees; ++tree) {
            if (principal_dn == NULL) {
                LDAP_SEARCH_1(subtreelist[tree], ldap_context->lrparams->search_scope, filter, principal_attributes, IGNORE_STATUS);
            } else {
                LDAP_SEARCH_1(principal_dn, LDAP_SCOPE_BASE, filter, principal_attributes, IGNORE_STATUS);
            }
            if (st == LDAP_SUCCESS) {
                numlentries = ldap_count_entries(ld, result);
                if (numlentries > 1) {
                    free(filter);
                    st = EINVAL;
                    k5_setmsg(context, st,
                              _("operation can not continue, more than one "
                                "entry with principal name \"%s\" found"),
                              user);
                    goto cleanup;
                } else if (numlentries == 1) {
                    found_entry = TRUE;
                    if (principal_dn == NULL) {
                        ent = ldap_first_entry(ld, result);
                        if (ent != NULL) {
                            if ((principal_dn = ldap_get_dn(ld, ent)) == NULL) {
                                ldap_get_option (ld, LDAP_OPT_RESULT_CODE, &st);
                                st = set_ldap_error (context, st, 0);
                                free(filter);
                                goto cleanup;
                            }
                        }
                    }
                }
            } else if (st != LDAP_NO_SUCH_OBJECT) {
                st = set_ldap_error (context, st, 0);
                free(filter);
                goto cleanup;
            }
            ldap_msgfree(result);
            result = NULL;
        }  
        free(filter);
        if (found_entry == FALSE && principal_dn != NULL) {
            create_standalone_prinicipal = TRUE;
            standalone_principal_dn = strdup(principal_dn);
            CHECK_NULL(standalone_principal_dn);
        }
    }  
    if (principal_dn == NULL && xargs.dn == NULL) {  
        if (entry->princ->length == 2 && entry->princ->data[0].length == strlen("krbtgt") &&
            strncmp(entry->princ->data[0].data, "krbtgt", entry->princ->data[0].length) == 0) {
            subtree = strdup(ldap_context->lrparams->realmdn);
        } else if (xargs.containerdn) {
            if ((st=checkattributevalue(ld, xargs.containerdn, NULL, NULL, NULL)) != 0) {
                if (st == KRB5_KDB_NOENTRY || st == KRB5_KDB_CONSTRAINT_VIOLATION) {
                    int ost = st;
                    st = EINVAL;
                    k5_prependmsg(context, ost, st, _("'%s' not found"),
                                  xargs.containerdn);
                }
                goto cleanup;
            }
            subtree = strdup(xargs.containerdn);
        } else if (ldap_context->lrparams->containerref && strlen(ldap_context->lrparams->containerref) != 0) {
            subtree = strdup(ldap_context->lrparams->containerref);
        } else {
            subtree = strdup(ldap_context->lrparams->realmdn);
        }
        CHECK_NULL(subtree);
        if (asprintf(&standalone_principal_dn, "krbprincipalname=%s,%s",
                     filtuser, subtree) < 0)
            standalone_principal_dn = NULL;
        CHECK_NULL(standalone_principal_dn);
        create_standalone_prinicipal = TRUE;
        free(subtree);
        subtree = NULL;
    }
    if (xargs.dn_from_kbd == TRUE) {
        int              dnlen=0, subtreelen=0;
        char             *dn=NULL;
        krb5_boolean     outofsubtree=TRUE;
        if (xargs.dn != NULL) {
            dn = xargs.dn;
        } else if (xargs.linkdn != NULL) {
            dn = xargs.linkdn;
        } else if (standalone_principal_dn != NULL) {
            dn = standalone_principal_dn;
        }
        if (subtreelist == NULL) {
            st = krb5_get_subtree_info(ldap_context, &subtreelist, &ntrees);
            if (st)
                goto cleanup;
        }
        for (tre=0; tre<ntrees; ++tre) {
            if (subtreelist[tre] == NULL || strlen(subtreelist[tre]) == 0) {
                outofsubtree = FALSE;
                break;
            } else {
                dnlen = strlen (dn);
                subtreelen = strlen(subtreelist[tre]);
                if ((dnlen >= subtreelen) && (strcasecmp((dn + dnlen - subtreelen), subtreelist[tre]) == 0)) {
                    outofsubtree = FALSE;
                    break;
                }
            }
        }
        if (outofsubtree == TRUE) {
            st = EINVAL;
            k5_setmsg(context, st, _("DN is out of the realm subtree"));
            goto cleanup;
        }
        if (standalone_principal_dn == NULL) {
            char  *attributes[]={"krbticketpolicyreference", "krbprincipalname", NULL};
            ldap_msgfree(result);
            result = NULL;
            LDAP_SEARCH_1(dn, LDAP_SCOPE_BASE, 0, attributes, IGNORE_STATUS);
            if (st == LDAP_SUCCESS) {
                ent = ldap_first_entry(ld, result);
                if (ent != NULL) {
                    if ((values=ldap_get_values(ld, ent, "krbticketpolicyreference")) != NULL) {
                        ldap_value_free(values);
                    }
                    if ((values=ldap_get_values(ld, ent, "krbprincipalname")) != NULL) {
                        krb_identity_exists = TRUE;
                        ldap_value_free(values);
                    }
                }
            } else {
                st = set_ldap_error(context, st, OP_SEARCH);
                goto cleanup;
            }
        }
    }
    if (xargs.dn != NULL && krb_identity_exists == TRUE) {
        st = EINVAL;
        snprintf(errbuf, sizeof(errbuf),
                 _("ldap object is already kerberized"));
        k5_setmsg(context, st, "%s", errbuf);
        goto cleanup;
    }
    if (xargs.linkdn != NULL) {
        if (optype == MODIFY_PRINCIPAL &&
            kerberos_principal_object_type != KDB_STANDALONE_PRINCIPAL_OBJECT) {
            st = EINVAL;
            snprintf(errbuf, sizeof(errbuf),
                     _("link information can not be set/updated as the "
                       "kerberos principal belongs to an ldap object"));
            k5_setmsg(context, st, "%s", errbuf);
            goto cleanup;
        }
        {
            char **linkdns=NULL;
            int  j=0;
            if ((st=krb5_get_linkdn(context, entry, &linkdns)) != 0) {
                snprintf(errbuf, sizeof(errbuf),
                         _("Failed getting object references"));
                k5_setmsg(context, st, "%s", errbuf);
                goto cleanup;
            }
            if (linkdns != NULL) {
                st = EINVAL;
                snprintf(errbuf, sizeof(errbuf),
                         _("kerberos principal is already linked to a ldap "
                           "object"));
                k5_setmsg(context, st, "%s", errbuf);
                for (j=0; linkdns[j] != NULL; ++j)
                    free (linkdns[j]);
                free (linkdns);
                goto cleanup;
            }
        }
        establish_links = TRUE;
    }
    if (entry->mask & KADM5_LAST_SUCCESS) {
        memset(strval, 0, sizeof(strval));
        if ((strval[0]=getstringtime(entry->last_success)) == NULL)
            goto cleanup;
        if ((st=krb5_add_str_mem_ldap_mod(&mods, "krbLastSuccessfulAuth", LDAP_MOD_REPLACE, strval)) != 0) {
            free (strval[0]);
            goto cleanup;
        }
        free (strval[0]);
    }
    if (entry->mask & KADM5_LAST_FAILED) {
        memset(strval, 0, sizeof(strval));
        if ((strval[0]=getstringtime(entry->last_failed)) == NULL)
            goto cleanup;
        if ((st=krb5_add_str_mem_ldap_mod(&mods, "krbLastFailedAuth", LDAP_MOD_REPLACE, strval)) != 0) {
            free (strval[0]);
            goto cleanup;
        }
        free(strval[0]);
    }
    if (entry->mask & KADM5_FAIL_AUTH_COUNT) {
        krb5_kvno fail_auth_count;
        fail_auth_count = entry->fail_auth_count;
        if (entry->mask & KADM5_FAIL_AUTH_COUNT_INCREMENT)
            fail_auth_count++;
        st = krb5_add_int_mem_ldap_mod(&mods, "krbLoginFailedCount",
                                       LDAP_MOD_REPLACE,
                                       fail_auth_count);
        if (st != 0)
            goto cleanup;
    } else if (entry->mask & KADM5_FAIL_AUTH_COUNT_INCREMENT) {
        int attr_mask = 0;
        krb5_boolean has_fail_count;
        st = krb5_get_attributes_mask(context, entry, &attr_mask);
        if (st != 0)
            goto cleanup;
        has_fail_count = ((attr_mask & KDB_FAIL_AUTH_COUNT_ATTR) != 0);
#ifdef LDAP_MOD_INCREMENT
        if (ldap_server_handle->server_info->modify_increment &&
            has_fail_count) {
            st = krb5_add_int_mem_ldap_mod(&mods, "krbLoginFailedCount",
                                           LDAP_MOD_INCREMENT, 1);
            if (st != 0)
                goto cleanup;
        } else {
#endif  
            if (has_fail_count) {
                st = krb5_add_int_mem_ldap_mod(&mods,
                                               "krbLoginFailedCount",
                                               LDAP_MOD_DELETE,
                                               entry->fail_auth_count);
                if (st != 0)
                    goto cleanup;
            }
            st = krb5_add_int_mem_ldap_mod(&mods, "krbLoginFailedCount",
                                           LDAP_MOD_ADD,
                                           entry->fail_auth_count + 1);
            if (st != 0)
                goto cleanup;
#ifdef LDAP_MOD_INCREMENT
        }
#endif
    } else if (optype == ADD_PRINCIPAL) {
        st = krb5_add_int_mem_ldap_mod(&mods, "krbLoginFailedCount",
                                       LDAP_MOD_ADD, 0);
    }
    if (entry->mask & KADM5_MAX_LIFE) {
        if ((st=krb5_add_int_mem_ldap_mod(&mods, "krbmaxticketlife", LDAP_MOD_REPLACE, entry->max_life)) != 0)
            goto cleanup;
    }
    if (entry->mask & KADM5_MAX_RLIFE) {
        if ((st=krb5_add_int_mem_ldap_mod(&mods, "krbmaxrenewableage", LDAP_MOD_REPLACE,
                                          entry->max_renewable_life)) != 0)
            goto cleanup;
    }
    if (entry->mask & KADM5_ATTRIBUTES) {
        if ((st=krb5_add_int_mem_ldap_mod(&mods, "krbticketflags", LDAP_MOD_REPLACE,
                                          entry->attributes)) != 0)
            goto cleanup;
    }
    if (entry->mask & KADM5_PRINCIPAL) {
        memset(strval, 0, sizeof(strval));
        strval[0] = user;
        if ((st=krb5_add_str_mem_ldap_mod(&mods, "krbprincipalname", LDAP_MOD_REPLACE, strval)) != 0)
            goto cleanup;
    }
    if (entry->mask & KADM5_PRINC_EXPIRE_TIME) {
        memset(strval, 0, sizeof(strval));
        if ((strval[0]=getstringtime(entry->expiration)) == NULL)
            goto cleanup;
        if ((st=krb5_add_str_mem_ldap_mod(&mods, "krbprincipalexpiration", LDAP_MOD_REPLACE, strval)) != 0) {
            free (strval[0]);
            goto cleanup;
        }
        free (strval[0]);
    }
    if (entry->mask & KADM5_PW_EXPIRATION) {
        memset(strval, 0, sizeof(strval));
        if ((strval[0]=getstringtime(entry->pw_expiration)) == NULL)
            goto cleanup;
        if ((st=krb5_add_str_mem_ldap_mod(&mods, "krbpasswordexpiration",
                                          LDAP_MOD_REPLACE,
                                          strval)) != 0) {
            free (strval[0]);
            goto cleanup;
        }
        free (strval[0]);
    }
    if (entry->mask & KADM5_POLICY) {
        memset(&princ_ent, 0, sizeof(princ_ent));
        for (tl_data=entry->tl_data; tl_data; tl_data=tl_data->tl_data_next) {
            if (tl_data->tl_data_type == KRB5_TL_KADM_DATA) {
                if ((st = krb5_lookup_tl_kadm_data(tl_data, &princ_ent)) != 0) {
                    goto cleanup;
                }
                break;
            }
        }
        if (princ_ent.aux_attributes & KADM5_POLICY) {
            memset(strval, 0, sizeof(strval));
            if ((st = krb5_ldap_name_to_policydn (context, princ_ent.policy, &polname)) != 0)
                goto cleanup;
            strval[0] = polname;
            if ((st=krb5_add_str_mem_ldap_mod(&mods, "krbpwdpolicyreference", LDAP_MOD_REPLACE, strval)) != 0)
                goto cleanup;
        } else {
            st = EINVAL;
            k5_setmsg(context, st, "Password policy value null");
            goto cleanup;
        }
    } else if (entry->mask & KADM5_LOAD && found_entry == TRUE) {
        if ((st=krb5_add_str_mem_ldap_mod(&mods, "krbpwdpolicyreference", LDAP_MOD_REPLACE, NULL)) != 0)
            goto cleanup;
    }
    if (entry->mask & KADM5_POLICY_CLR) {
        if ((st=krb5_add_str_mem_ldap_mod(&mods, "krbpwdpolicyreference", LDAP_MOD_DELETE, NULL)) != 0)
            goto cleanup;
    }
    if (entry->mask & KADM5_KEY_DATA || entry->mask & KADM5_KVNO) {
        krb5_kvno mkvno;
        if ((st=krb5_dbe_lookup_mkvno(context, entry, &mkvno)) != 0)
            goto cleanup;
        bersecretkey = krb5_encode_krbsecretkey (entry->key_data,
                                                 entry->n_key_data, mkvno);
        if ((st=krb5_add_ber_mem_ldap_mod(&mods, "krbprincipalkey",
                                          LDAP_MOD_REPLACE | LDAP_MOD_BVALUES, bersecretkey)) != 0)
            goto cleanup;
        if (!(entry->mask & KADM5_PRINCIPAL)) {
            memset(strval, 0, sizeof(strval));
            if ((strval[0]=getstringtime(entry->pw_expiration)) == NULL)
                goto cleanup;
            if ((st=krb5_add_str_mem_ldap_mod(&mods,
                                              "krbpasswordexpiration",
                                              LDAP_MOD_REPLACE, strval)) != 0) {
                free (strval[0]);
                goto cleanup;
            }
            free (strval[0]);
        }
        {
            krb5_timestamp last_pw_changed;
            if ((st=krb5_dbe_lookup_last_pwd_change(context, entry,
                                                    &last_pw_changed)) != 0)
                goto cleanup;
            memset(strval, 0, sizeof(strval));
            if ((strval[0] = getstringtime(last_pw_changed)) == NULL)
                goto cleanup;
            if ((st=krb5_add_str_mem_ldap_mod(&mods, "krbLastPwdChange",
                                              LDAP_MOD_REPLACE, strval)) != 0) {
                free (strval[0]);
                goto cleanup;
            }
            free (strval[0]);
        }
    }  
    if (entry->tl_data != NULL) {
        int count = 0;
        struct berval **ber_tl_data = NULL;
        krb5_tl_data *ptr;
        krb5_timestamp unlock_time;
        for (ptr = entry->tl_data; ptr != NULL; ptr = ptr->tl_data_next) {
            if (ptr->tl_data_type == KRB5_TL_LAST_PWD_CHANGE
#ifdef SECURID
                || ptr->tl_data_type == KRB5_TL_DB_ARGS
#endif
                || ptr->tl_data_type == KRB5_TL_KADM_DATA
                || ptr->tl_data_type == KDB_TL_USER_INFO
                || ptr->tl_data_type == KRB5_TL_CONSTRAINED_DELEGATION_ACL
                || ptr->tl_data_type == KRB5_TL_LAST_ADMIN_UNLOCK)
                continue;
            count++;
        }
        if (count != 0) {
            int j;
            ber_tl_data = (struct berval **) calloc (count + 1,
                                                     sizeof (struct berval*));
            if (ber_tl_data == NULL) {
                st = ENOMEM;
                goto cleanup;
            }
            for (j = 0, ptr = entry->tl_data; ptr != NULL; ptr = ptr->tl_data_next) {
                if (ptr->tl_data_type == KRB5_TL_LAST_PWD_CHANGE
#ifdef SECURID
                    || ptr->tl_data_type == KRB5_TL_DB_ARGS
#endif
                    || ptr->tl_data_type == KRB5_TL_KADM_DATA
                    || ptr->tl_data_type == KDB_TL_USER_INFO
                    || ptr->tl_data_type == KRB5_TL_CONSTRAINED_DELEGATION_ACL
                    || ptr->tl_data_type == KRB5_TL_LAST_ADMIN_UNLOCK)
                    continue;
                if ((st = tl_data2berval (ptr, &ber_tl_data[j])) != 0)
                    break;
                j++;
            }
            if (st == 0) {
                ber_tl_data[count] = NULL;
                st=krb5_add_ber_mem_ldap_mod(&mods, "krbExtraData",
                                             LDAP_MOD_REPLACE |
                                             LDAP_MOD_BVALUES, ber_tl_data);
            }
            for (j = 0; ber_tl_data[j] != NULL; j++) {
                free(ber_tl_data[j]->bv_val);
                free(ber_tl_data[j]);
            }
            free(ber_tl_data);
            if (st != 0)
                goto cleanup;
        }
        if ((st=krb5_dbe_lookup_last_admin_unlock(context, entry,
                                                  &unlock_time)) != 0)
            goto cleanup;
        if (unlock_time != 0) {
            memset(strval, 0, sizeof(strval));
            if ((strval[0] = getstringtime(unlock_time)) == NULL)
                goto cleanup;
            if ((st=krb5_add_str_mem_ldap_mod(&mods, "krbLastAdminUnlock",
                                              LDAP_MOD_REPLACE, strval)) != 0) {
                free (strval[0]);
                goto cleanup;
            }
            free (strval[0]);
        }
    }
    if (xargs.tktpolicydn != NULL) {
        int tmask=0;
        if (strlen(xargs.tktpolicydn) != 0) {
            st = checkattributevalue(ld, xargs.tktpolicydn, "objectclass", policyclass, &tmask);
            CHECK_CLASS_VALIDITY(st, tmask, _("ticket policy object value: "));
            strval[0] = xargs.tktpolicydn;
            strval[1] = NULL;
            if ((st=krb5_add_str_mem_ldap_mod(&mods, "krbticketpolicyreference", LDAP_MOD_REPLACE, strval)) != 0)
                goto cleanup;
        } else {
            if ((st=krb5_add_str_mem_ldap_mod(&mods, "krbticketpolicyreference", LDAP_MOD_DELETE, NULL)) != 0)
                goto cleanup;
        }
    }
    if (establish_links == TRUE) {
        memset(strval, 0, sizeof(strval));
        strval[0] = xargs.linkdn;
        if ((st=krb5_add_str_mem_ldap_mod(&mods, "krbObjectReferences", LDAP_MOD_REPLACE, strval)) != 0)
            goto cleanup;
    }
    if (mods == NULL)
        goto cleanup;
    if (create_standalone_prinicipal == TRUE) {
        memset(strval, 0, sizeof(strval));
        strval[0] = "krbprincipal";
        strval[1] = "krbprincipalaux";
        strval[2] = "krbTicketPolicyAux";
        if ((st=krb5_add_str_mem_ldap_mod(&mods, "objectclass", LDAP_MOD_ADD, strval)) != 0)
            goto cleanup;
        st = ldap_add_ext_s(ld, standalone_principal_dn, mods, NULL, NULL);
        if (st == LDAP_ALREADY_EXISTS && entry->mask & KADM5_LOAD) {
            st = ldap_delete_ext_s(ld, standalone_principal_dn, NULL, NULL);
            if (st != LDAP_SUCCESS) {
                snprintf(errbuf, sizeof(errbuf),
                         _("Principal delete failed (trying to replace "
                           "entry): %s"), ldap_err2string(st));
                st = translate_ldap_error (st, OP_ADD);
                k5_setmsg(context, st, "%s", errbuf);
                goto cleanup;
            } else {
                st = ldap_add_ext_s(ld, standalone_principal_dn, mods, NULL, NULL);
            }
        }
        if (st != LDAP_SUCCESS) {
            snprintf(errbuf, sizeof(errbuf), _("Principal add failed: %s"),
                     ldap_err2string(st));
            st = translate_ldap_error (st, OP_ADD);
            k5_setmsg(context, st, "%s", errbuf);
            goto cleanup;
        }
    } else {
        {
            char *attrvalues[] = {"krbprincipalaux", "krbTicketPolicyAux", NULL};
            int p, q, r=0, amask=0;
            if ((st=checkattributevalue(ld, (xargs.dn) ? xargs.dn : principal_dn,
                                        "objectclass", attrvalues, &amask)) != 0)
                goto cleanup;
            memset(strval, 0, sizeof(strval));
            for (p=1, q=0; p<=2; p<<=1, ++q) {
                if ((p & amask) == 0)
                    strval[r++] = attrvalues[q];
            }
            if (r != 0) {
                if ((st=krb5_add_str_mem_ldap_mod(&mods, "objectclass", LDAP_MOD_ADD, strval)) != 0)
                    goto cleanup;
            }
        }
        if (xargs.dn != NULL)
            st=ldap_modify_ext_s(ld, xargs.dn, mods, NULL, NULL);
        else
            st = ldap_modify_ext_s(ld, principal_dn, mods, NULL, NULL);
        if (st != LDAP_SUCCESS) {
            snprintf(errbuf, sizeof(errbuf), _("User modification failed: %s"),
                     ldap_err2string(st));
            st = translate_ldap_error (st, OP_MOD);
            k5_setmsg(context, st, "%s", errbuf);
            goto cleanup;
        }
        if (entry->mask & KADM5_FAIL_AUTH_COUNT_INCREMENT)
            entry->fail_auth_count++;
    }
cleanup:
    if (user)
        free(user);
    if (filtuser)
        free(filtuser);
    free_xargs(xargs);
    if (standalone_principal_dn)
        free(standalone_principal_dn);
    if (principal_dn)
        free (principal_dn);
    if (polname != NULL)
        free(polname);
    for (tre = 0; tre < ntrees; tre++)
        free(subtreelist[tre]);
    free(subtreelist);
    if (subtree)
        free (subtree);
    if (bersecretkey) {
        for (l=0; bersecretkey[l]; ++l) {
            if (bersecretkey[l]->bv_val)
                free (bersecretkey[l]->bv_val);
            free (bersecretkey[l]);
        }
        free (bersecretkey);
    }
    if (keys)
        free (keys);
    ldap_mods_free(mods, 1);
    ldap_osa_free_princ_ent(&princ_ent);
    ldap_msgfree(result);
    krb5_ldap_put_handle_to_pool(ldap_context, ldap_server_handle);
    return(st);
}