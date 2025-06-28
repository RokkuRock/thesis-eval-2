int __kvm_set_memory_region(struct kvm *kvm,
			    struct kvm_userspace_memory_region *mem,
			    int user_alloc)
{
	int r;
	gfn_t base_gfn;
	unsigned long npages;
	unsigned long i;
	struct kvm_memory_slot *memslot;
	struct kvm_memory_slot old, new;
	struct kvm_memslots *slots, *old_memslots;
	r = -EINVAL;
	if (mem->memory_size & (PAGE_SIZE - 1))
		goto out;
	if (mem->guest_phys_addr & (PAGE_SIZE - 1))
		goto out;
	if (user_alloc && (mem->userspace_addr & (PAGE_SIZE - 1)))
		goto out;
	if (mem->slot >= KVM_MEMORY_SLOTS + KVM_PRIVATE_MEM_SLOTS)
		goto out;
	if (mem->guest_phys_addr + mem->memory_size < mem->guest_phys_addr)
		goto out;
	memslot = &kvm->memslots->memslots[mem->slot];
	base_gfn = mem->guest_phys_addr >> PAGE_SHIFT;
	npages = mem->memory_size >> PAGE_SHIFT;
	r = -EINVAL;
	if (npages > KVM_MEM_MAX_NR_PAGES)
		goto out;
	if (!npages)
		mem->flags &= ~KVM_MEM_LOG_DIRTY_PAGES;
	new = old = *memslot;
	new.id = mem->slot;
	new.base_gfn = base_gfn;
	new.npages = npages;
	new.flags = mem->flags;
	r = -EINVAL;
	if (npages && old.npages && npages != old.npages)
		goto out_free;
	r = -EEXIST;
	for (i = 0; i < KVM_MEMORY_SLOTS; ++i) {
		struct kvm_memory_slot *s = &kvm->memslots->memslots[i];
		if (s == memslot || !s->npages)
			continue;
		if (!((base_gfn + npages <= s->base_gfn) ||
		      (base_gfn >= s->base_gfn + s->npages)))
			goto out_free;
	}
	if (!(new.flags & KVM_MEM_LOG_DIRTY_PAGES))
		new.dirty_bitmap = NULL;
	r = -ENOMEM;
#ifndef CONFIG_S390
	if (npages && !new.rmap) {
		new.rmap = vzalloc(npages * sizeof(*new.rmap));
		if (!new.rmap)
			goto out_free;
		new.user_alloc = user_alloc;
		new.userspace_addr = mem->userspace_addr;
	}
	if (!npages)
		goto skip_lpage;
	for (i = 0; i < KVM_NR_PAGE_SIZES - 1; ++i) {
		unsigned long ugfn;
		unsigned long j;
		int lpages;
		int level = i + 2;
		(void)level;
		if (new.lpage_info[i])
			continue;
		lpages = 1 + ((base_gfn + npages - 1)
			     >> KVM_HPAGE_GFN_SHIFT(level));
		lpages -= base_gfn >> KVM_HPAGE_GFN_SHIFT(level);
		new.lpage_info[i] = vzalloc(lpages * sizeof(*new.lpage_info[i]));
		if (!new.lpage_info[i])
			goto out_free;
		if (base_gfn & (KVM_PAGES_PER_HPAGE(level) - 1))
			new.lpage_info[i][0].write_count = 1;
		if ((base_gfn+npages) & (KVM_PAGES_PER_HPAGE(level) - 1))
			new.lpage_info[i][lpages - 1].write_count = 1;
		ugfn = new.userspace_addr >> PAGE_SHIFT;
		if ((base_gfn ^ ugfn) & (KVM_PAGES_PER_HPAGE(level) - 1) ||
		    !largepages_enabled)
			for (j = 0; j < lpages; ++j)
				new.lpage_info[i][j].write_count = 1;
	}
skip_lpage:
	if ((new.flags & KVM_MEM_LOG_DIRTY_PAGES) && !new.dirty_bitmap) {
		if (kvm_create_dirty_bitmap(&new) < 0)
			goto out_free;
	}
#else   
	new.user_alloc = user_alloc;
	if (user_alloc)
		new.userspace_addr = mem->userspace_addr;
#endif  
	if (!npages) {
		r = -ENOMEM;
		slots = kzalloc(sizeof(struct kvm_memslots), GFP_KERNEL);
		if (!slots)
			goto out_free;
		memcpy(slots, kvm->memslots, sizeof(struct kvm_memslots));
		if (mem->slot >= slots->nmemslots)
			slots->nmemslots = mem->slot + 1;
		slots->generation++;
		slots->memslots[mem->slot].flags |= KVM_MEMSLOT_INVALID;
		old_memslots = kvm->memslots;
		rcu_assign_pointer(kvm->memslots, slots);
		synchronize_srcu_expedited(&kvm->srcu);
		kvm_arch_flush_shadow(kvm);
		kfree(old_memslots);
	}
	r = kvm_arch_prepare_memory_region(kvm, &new, old, mem, user_alloc);
	if (r)
		goto out_free;
	if (npages) {
		r = kvm_iommu_map_pages(kvm, &new);
		if (r)
			goto out_free;
	}
	r = -ENOMEM;
	slots = kzalloc(sizeof(struct kvm_memslots), GFP_KERNEL);
	if (!slots)
		goto out_free;
	memcpy(slots, kvm->memslots, sizeof(struct kvm_memslots));
	if (mem->slot >= slots->nmemslots)
		slots->nmemslots = mem->slot + 1;
	slots->generation++;
	if (!npages) {
		new.rmap = NULL;
		new.dirty_bitmap = NULL;
		for (i = 0; i < KVM_NR_PAGE_SIZES - 1; ++i)
			new.lpage_info[i] = NULL;
	}
	slots->memslots[mem->slot] = new;
	old_memslots = kvm->memslots;
	rcu_assign_pointer(kvm->memslots, slots);
	synchronize_srcu_expedited(&kvm->srcu);
	kvm_arch_commit_memory_region(kvm, mem, old, user_alloc);
	kvm_free_physmem_slot(&old, &new);
	kfree(old_memslots);
	return 0;
out_free:
	kvm_free_physmem_slot(&new, &old);
out:
	return r;
}