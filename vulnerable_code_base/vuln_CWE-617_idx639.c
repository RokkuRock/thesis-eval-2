pci_emul_alloc_resource(uint64_t *baseptr, uint64_t limit, uint64_t size,
			uint64_t *addr)
{
	uint64_t base;
	assert((size & (size - 1)) == 0);	 
	base = roundup2(*baseptr, size);
	if (base + size <= limit) {
		*addr = base;
		*baseptr = base + size;
		return 0;
	} else
		return -1;
}