int bus_verify_polkit_async(
                sd_bus_message *call,
                int capability,
                const char *action,
                const char **details,
                bool interactive,
                uid_t good_user,
                Hashmap **registry,
                sd_bus_error *ret_error) {
#if ENABLE_POLKIT
        _cleanup_(sd_bus_message_unrefp) sd_bus_message *pk = NULL;
        AsyncPolkitQuery *q;
        const char *sender;
        sd_bus_message_handler_t callback;
        void *userdata;
        int c;
#endif
        int r;
        assert(call);
        assert(action);
        assert(registry);
        r = check_good_user(call, good_user);
        if (r != 0)
                return r;
#if ENABLE_POLKIT
        q = hashmap_get(*registry, call);
        if (q) {
                int authorized, challenge;
                assert(q->reply);
                if (!streq(q->action, action) ||
                    !strv_equal(q->details, (char**) details))
                        return -ESTALE;
                if (sd_bus_message_is_method_error(q->reply, NULL)) {
                        const sd_bus_error *e;
                        e = sd_bus_message_get_error(q->reply);
                        if (sd_bus_error_has_name(e, SD_BUS_ERROR_SERVICE_UNKNOWN) ||
                            sd_bus_error_has_name(e, SD_BUS_ERROR_NAME_HAS_NO_OWNER))
                                return -EACCES;
                        sd_bus_error_copy(ret_error, e);
                        return -sd_bus_error_get_errno(e);
                }
                r = sd_bus_message_enter_container(q->reply, 'r', "bba{ss}");
                if (r >= 0)
                        r = sd_bus_message_read(q->reply, "bb", &authorized, &challenge);
                if (r < 0)
                        return r;
                if (authorized)
                        return 1;
                if (challenge)
                        return sd_bus_error_set(ret_error, SD_BUS_ERROR_INTERACTIVE_AUTHORIZATION_REQUIRED, "Interactive authentication required.");
                return -EACCES;
        }
#endif
        r = sd_bus_query_sender_privilege(call, capability);
        if (r < 0)
                return r;
        else if (r > 0)
                return 1;
#if ENABLE_POLKIT
        if (sd_bus_get_current_message(call->bus) != call)
                return -EINVAL;
        callback = sd_bus_get_current_handler(call->bus);
        if (!callback)
                return -EINVAL;
        userdata = sd_bus_get_current_userdata(call->bus);
        sender = sd_bus_message_get_sender(call);
        if (!sender)
                return -EBADMSG;
        c = sd_bus_message_get_allow_interactive_authorization(call);
        if (c < 0)
                return c;
        if (c > 0)
                interactive = true;
        r = hashmap_ensure_allocated(registry, NULL);
        if (r < 0)
                return r;
        r = sd_bus_message_new_method_call(
                        call->bus,
                        &pk,
                        "org.freedesktop.PolicyKit1",
                        "/org/freedesktop/PolicyKit1/Authority",
                        "org.freedesktop.PolicyKit1.Authority",
                        "CheckAuthorization");
        if (r < 0)
                return r;
        r = sd_bus_message_append(
                        pk,
                        "(sa{sv})s",
                        "system-bus-name", 1, "name", "s", sender,
                        action);
        if (r < 0)
                return r;
        r = bus_message_append_strv_key_value(pk, details);
        if (r < 0)
                return r;
        r = sd_bus_message_append(pk, "us", interactive, NULL);
        if (r < 0)
                return r;
        q = new(AsyncPolkitQuery, 1);
        if (!q)
                return -ENOMEM;
        *q = (AsyncPolkitQuery) {
                .request = sd_bus_message_ref(call),
                .callback = callback,
                .userdata = userdata,
        };
        q->action = strdup(action);
        if (!q->action) {
                async_polkit_query_free(q);
                return -ENOMEM;
        }
        q->details = strv_copy((char**) details);
        if (!q->details) {
                async_polkit_query_free(q);
                return -ENOMEM;
        }
        r = hashmap_put(*registry, call, q);
        if (r < 0) {
                async_polkit_query_free(q);
                return r;
        }
        q->registry = *registry;
        r = sd_bus_call_async(call->bus, &q->slot, pk, async_polkit_callback, q, 0);
        if (r < 0) {
                async_polkit_query_free(q);
                return r;
        }
        return 0;
#endif
        return -EACCES;
}