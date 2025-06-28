static inline struct anon_vma *anon_vma_alloc(void)
{
	struct anon_vma *anon_vma;
	anon_vma = kmem_cache_alloc(anon_vma_cachep, GFP_KERNEL);
	if (anon_vma) {
		atomic_set(&anon_vma->refcount, 1);
		anon_vma->degree = 1;	 
		anon_vma->parent = anon_vma;
		anon_vma->root = anon_vma;
	}
	return anon_vma;
}