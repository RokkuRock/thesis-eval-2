TRIO_PUBLIC trio_pointer_t trio_register TRIO_ARGS2((callback, name), trio_callback_t callback,
                                                    TRIO_CONST char* name)
{
	trio_userdef_t* def;
	trio_userdef_t* prev = NULL;
	if (callback == NULL)
		return NULL;
	if (name)
	{
		if (name[0] == ':')
		{
			if (trio_equal(name, ":enter"))
			{
				internalEnterCriticalRegion = callback;
			}
			else if (trio_equal(name, ":leave"))
			{
				internalLeaveCriticalRegion = callback;
			}
			return NULL;
		}
		if (trio_length(name) >= MAX_USER_NAME)
			return NULL;
		def = TrioFindNamespace(name, &prev);
		if (def)
			return NULL;
	}
	def = (trio_userdef_t*)TRIO_MALLOC(sizeof(trio_userdef_t));
	if (def)
	{
		if (internalEnterCriticalRegion)
			(void)internalEnterCriticalRegion(NULL);
		if (name)
		{
			if (prev == NULL)
				internalUserDef = def;
			else
				prev->next = def;
		}
		def->callback = callback;
		def->name = (name == NULL) ? NULL : trio_duplicate(name);
		def->next = NULL;
		if (internalLeaveCriticalRegion)
			(void)internalLeaveCriticalRegion(NULL);
	}
	return (trio_pointer_t)def;
}