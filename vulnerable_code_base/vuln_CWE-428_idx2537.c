pkcs11_register_provider(char *provider_id, char *pin,
    struct sshkey ***keyp, char ***labelsp,
    struct pkcs11_provider **providerp, CK_ULONG user)
{
	int nkeys, need_finalize = 0;
	int ret = -1;
	struct pkcs11_provider *p = NULL;
	void *handle = NULL;
	CK_RV (*getfunctionlist)(CK_FUNCTION_LIST **);
	CK_RV rv;
	CK_FUNCTION_LIST *f = NULL;
	CK_TOKEN_INFO *token;
	CK_ULONG i;
	if (providerp == NULL)
		goto fail;
	*providerp = NULL;
	if (keyp != NULL)
		*keyp = NULL;
	if (labelsp != NULL)
		*labelsp = NULL;
	if (pkcs11_provider_lookup(provider_id) != NULL) {
		debug_f("provider already registered: %s", provider_id);
		goto fail;
	}
	if ((handle = dlopen(provider_id, RTLD_NOW)) == NULL) {
		error("dlopen %s failed: %s", provider_id, dlerror());
		goto fail;
	}
	if ((getfunctionlist = dlsym(handle, "C_GetFunctionList")) == NULL) {
		error("dlsym(C_GetFunctionList) failed: %s", dlerror());
		goto fail;
	}
	p = xcalloc(1, sizeof(*p));
	p->name = xstrdup(provider_id);
	p->handle = handle;
	if ((rv = (*getfunctionlist)(&f)) != CKR_OK) {
		error("C_GetFunctionList for provider %s failed: %lu",
		    provider_id, rv);
		goto fail;
	}
	p->function_list = f;
	if ((rv = f->C_Initialize(NULL)) != CKR_OK) {
		error("C_Initialize for provider %s failed: %lu",
		    provider_id, rv);
		goto fail;
	}
	need_finalize = 1;
	if ((rv = f->C_GetInfo(&p->info)) != CKR_OK) {
		error("C_GetInfo for provider %s failed: %lu",
		    provider_id, rv);
		goto fail;
	}
	rmspace(p->info.manufacturerID, sizeof(p->info.manufacturerID));
	rmspace(p->info.libraryDescription, sizeof(p->info.libraryDescription));
	debug("provider %s: manufacturerID <%s> cryptokiVersion %d.%d"
	    " libraryDescription <%s> libraryVersion %d.%d",
	    provider_id,
	    p->info.manufacturerID,
	    p->info.cryptokiVersion.major,
	    p->info.cryptokiVersion.minor,
	    p->info.libraryDescription,
	    p->info.libraryVersion.major,
	    p->info.libraryVersion.minor);
	if ((rv = f->C_GetSlotList(CK_TRUE, NULL, &p->nslots)) != CKR_OK) {
		error("C_GetSlotList failed: %lu", rv);
		goto fail;
	}
	if (p->nslots == 0) {
		debug_f("provider %s returned no slots", provider_id);
		ret = -SSH_PKCS11_ERR_NO_SLOTS;
		goto fail;
	}
	p->slotlist = xcalloc(p->nslots, sizeof(CK_SLOT_ID));
	if ((rv = f->C_GetSlotList(CK_TRUE, p->slotlist, &p->nslots))
	    != CKR_OK) {
		error("C_GetSlotList for provider %s failed: %lu",
		    provider_id, rv);
		goto fail;
	}
	p->slotinfo = xcalloc(p->nslots, sizeof(struct pkcs11_slotinfo));
	p->valid = 1;
	nkeys = 0;
	for (i = 0; i < p->nslots; i++) {
		token = &p->slotinfo[i].token;
		if ((rv = f->C_GetTokenInfo(p->slotlist[i], token))
		    != CKR_OK) {
			error("C_GetTokenInfo for provider %s slot %lu "
			    "failed: %lu", provider_id, (u_long)i, rv);
			continue;
		}
		if ((token->flags & CKF_TOKEN_INITIALIZED) == 0) {
			debug2_f("ignoring uninitialised token in "
			    "provider %s slot %lu", provider_id, (u_long)i);
			continue;
		}
		rmspace(token->label, sizeof(token->label));
		rmspace(token->manufacturerID, sizeof(token->manufacturerID));
		rmspace(token->model, sizeof(token->model));
		rmspace(token->serialNumber, sizeof(token->serialNumber));
		debug("provider %s slot %lu: label <%s> manufacturerID <%s> "
		    "model <%s> serial <%s> flags 0x%lx",
		    provider_id, (unsigned long)i,
		    token->label, token->manufacturerID, token->model,
		    token->serialNumber, token->flags);
		if ((ret = pkcs11_open_session(p, i, pin, user)) != 0 ||
		    keyp == NULL)
			continue;
		pkcs11_fetch_keys(p, i, keyp, labelsp, &nkeys);
		pkcs11_fetch_certs(p, i, keyp, labelsp, &nkeys);
		if (nkeys == 0 && !p->slotinfo[i].logged_in &&
		    pkcs11_interactive) {
			if (pkcs11_login_slot(p, &p->slotinfo[i],
			    CKU_USER) < 0) {
				error("login failed");
				continue;
			}
			pkcs11_fetch_keys(p, i, keyp, labelsp, &nkeys);
			pkcs11_fetch_certs(p, i, keyp, labelsp, &nkeys);
		}
	}
	*providerp = p;
	TAILQ_INSERT_TAIL(&pkcs11_providers, p, next);
	p->refcount++;	 
	return (nkeys);
fail:
	if (need_finalize && (rv = f->C_Finalize(NULL)) != CKR_OK)
		error("C_Finalize for provider %s failed: %lu",
		    provider_id, rv);
	if (p) {
		free(p->name);
		free(p->slotlist);
		free(p->slotinfo);
		free(p);
	}
	if (handle)
		dlclose(handle);
	if (ret > 0)
		ret = -1;
	return (ret);
}