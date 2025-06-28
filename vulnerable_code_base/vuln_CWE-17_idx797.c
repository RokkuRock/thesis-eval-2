void __init files_init(unsigned long mempages)
{ 
	unsigned long n;
	filp_cachep = kmem_cache_create("filp", sizeof(struct file), 0,
			SLAB_HWCACHE_ALIGN | SLAB_PANIC, NULL);
	n = (mempages * (PAGE_SIZE / 1024)) / 10;
	files_stat.max_files = max_t(unsigned long, n, NR_FILE);
	files_defer_init();
	lg_lock_init(&files_lglock, "files_lglock");
	percpu_counter_init(&nr_files, 0);
} 