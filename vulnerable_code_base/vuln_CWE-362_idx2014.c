static inline const unsigned char *fsnotify_oldname_init(const unsigned char *name)
{
	return kstrdup(name, GFP_KERNEL);
}