static inline void fsnotify_oldname_free(const unsigned char *old_name)
{
	kfree(old_name);
}