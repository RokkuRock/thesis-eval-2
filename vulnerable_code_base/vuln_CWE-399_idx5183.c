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
	r = check_memory_region_flags(mem);
	if (r)
		goto out;
	r = -EINVAL;
	if (mem->memory_size & (PAGE_SIZE - 1))
		goto out;
	if (mem->guest_phys_addr & (PAGE_SIZE - 1))
		goto out;
	if (user_alloc &&
	    ((mem->userspace_addr & (PAGE_SIZE - 1)) ||
	     !access_ok(VERIFY_WRITE,
			(void __user *)(unsigned long)mem->userspace_addr,
			mem->memory_size)))
		goto out;
	if (mem->slot >= KVM_MEM_SLOTS_NUM)
		goto out;
	if (mem->guest_phys_addr + mem->memory_size < mem->guest_phys_addr)
		goto out;
	memslot = id_to_memslot(kvm->memslots, mem->slot);
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
	if (npages && !old.npages) {
		new.user_alloc = user_alloc;
		new.userspace_addr = mem->userspace_addr;
		if (kvm_arch_create_memslot(&new, npages))
			goto out_free;
	}
	if ((new.flags & KVM_MEM_LOG_DIRTY_PAGES) && !new.dirty_bitmap) {
		if (kvm_create_dirty_bitmap(&new) < 0)
			goto out_free;
	}
	if (!npages) {
		struct kvm_memory_slot *slot;
		r = -ENOMEM;
		slots = kmemdup(kvm->memslots, sizeof(struct kvm_memslots),
				GFP_KERNEL);
		if (!slots)
			goto out_free;
		slot = id_to_memslot(slots, mem->slot);
		slot->flags |= KVM_MEMSLOT_INVALID;
		update_memslots(slots, NULL);
		old_memslots = kvm->memslots;
		rcu_assign_pointer(kvm->memslots, slots);
		synchronize_srcu_expedited(&kvm->srcu);
		kvm_arch_flush_shadow_memslot(kvm, slot);
		kfree(old_memslots);
	}
	r = kvm_arch_prepare_memory_region(kvm, &new, old, mem, user_alloc);
	if (r)
		goto out_free;
	if (npages) {
		r = kvm_iommu_map_pages(kvm, &new);
		if (r)
			goto out_free;
	} else
		kvm_iommu_unmap_pages(kvm, &old);
	r = -ENOMEM;
	slots = kmemdup(kvm->memslots, sizeof(struct kvm_memslots),
			GFP_KERNEL);
	if (!slots)
		goto out_free;
	if (!npages) {
		new.dirty_bitmap = NULL;
		memset(&new.arch, 0, sizeof(new.arch));
	}
	update_memslots(slots, &new);
	old_memslots = kvm->memslots;
	rcu_assign_pointer(kvm->memslots, slots);
	synchronize_srcu_expedited(&kvm->srcu);
	kvm_arch_commit_memory_region(kvm, mem, old, user_alloc);
	if (npages && old.base_gfn != mem->guest_phys_addr >> PAGE_SHIFT)
		kvm_arch_flush_shadow_all(kvm);
	kvm_free_physmem_slot(&old, &new);
	kfree(old_memslots);
	return 0;
out_free:
	kvm_free_physmem_slot(&new, &old);
out:
	return r;
}