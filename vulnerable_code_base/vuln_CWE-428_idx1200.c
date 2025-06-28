sshsk_open(const char *path)
{
	struct sshsk_provider *ret = NULL;
	uint32_t version;
	if (path == NULL || *path == '\0') {
		error("No FIDO SecurityKeyProvider specified");
		return NULL;
	}
	if ((ret = calloc(1, sizeof(*ret))) == NULL) {
		error_f("calloc failed");
		return NULL;
	}
	if ((ret->path = strdup(path)) == NULL) {
		error_f("strdup failed");
		goto fail;
	}
	if (strcasecmp(ret->path, "internal") == 0) {
		ret->sk_enroll = ssh_sk_enroll;
		ret->sk_sign = ssh_sk_sign;
		ret->sk_load_resident_keys = ssh_sk_load_resident_keys;
		return ret;
	}
	if ((ret->dlhandle = dlopen(path, RTLD_NOW)) == NULL) {
		error("Provider \"%s\" dlopen failed: %s", path, dlerror());
		goto fail;
	}
	if ((ret->sk_api_version = dlsym(ret->dlhandle,
	    "sk_api_version")) == NULL) {
		error("Provider \"%s\" dlsym(sk_api_version) failed: %s",
		    path, dlerror());
		goto fail;
	}
	version = ret->sk_api_version();
	debug_f("provider %s implements version 0x%08lx", ret->path,
	    (u_long)version);
	if ((version & SSH_SK_VERSION_MAJOR_MASK) != SSH_SK_VERSION_MAJOR) {
		error("Provider \"%s\" implements unsupported "
		    "version 0x%08lx (supported: 0x%08lx)",
		    path, (u_long)version, (u_long)SSH_SK_VERSION_MAJOR);
		goto fail;
	}
	if ((ret->sk_enroll = dlsym(ret->dlhandle, "sk_enroll")) == NULL) {
		error("Provider %s dlsym(sk_enroll) failed: %s",
		    path, dlerror());
		goto fail;
	}
	if ((ret->sk_sign = dlsym(ret->dlhandle, "sk_sign")) == NULL) {
		error("Provider \"%s\" dlsym(sk_sign) failed: %s",
		    path, dlerror());
		goto fail;
	}
	if ((ret->sk_load_resident_keys = dlsym(ret->dlhandle,
	    "sk_load_resident_keys")) == NULL) {
		error("Provider \"%s\" dlsym(sk_load_resident_keys) "
		    "failed: %s", path, dlerror());
		goto fail;
	}
	return ret;
fail:
	sshsk_free(ret);
	return NULL;
}