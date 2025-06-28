prepenv(const struct rule *rule)
{
	static const char *safeset[] = {
		"DISPLAY", "HOME", "LOGNAME", "MAIL",
		"PATH", "TERM", "USER", "USERNAME",
		NULL
	};
	struct env *env;
	env = createenv(rule);
	if (!(rule->options & KEEPENV))
		fillenv(env, safeset);
	if (rule->envlist)
		fillenv(env, rule->envlist);
	return flattenenv(env);
}