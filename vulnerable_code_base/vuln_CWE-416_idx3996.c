nvkm_vmm_get_locked(struct nvkm_vmm *vmm, bool getref, bool mapref, bool sparse,
		    u8 shift, u8 align, u64 size, struct nvkm_vma **pvma)
{
	const struct nvkm_vmm_page *page = &vmm->func->page[NVKM_VMA_PAGE_NONE];
	struct rb_node *node = NULL, *temp;
	struct nvkm_vma *vma = NULL, *tmp;
	u64 addr, tail;
	int ret;
	VMM_TRACE(vmm, "getref %d mapref %d sparse %d "
		       "shift: %d align: %d size: %016llx",
		  getref, mapref, sparse, shift, align, size);
	if (unlikely(!size || (!getref && !mapref && sparse))) {
		VMM_DEBUG(vmm, "args %016llx %d %d %d",
			  size, getref, mapref, sparse);
		return -EINVAL;
	}
	if (unlikely((getref || vmm->func->page_block) && !shift)) {
		VMM_DEBUG(vmm, "page size required: %d %016llx",
			  getref, vmm->func->page_block);
		return -EINVAL;
	}
	if (shift) {
		for (page = vmm->func->page; page->shift; page++) {
			if (shift == page->shift)
				break;
		}
		if (!page->shift || !IS_ALIGNED(size, 1ULL << page->shift)) {
			VMM_DEBUG(vmm, "page %d %016llx", shift, size);
			return -EINVAL;
		}
		align = max_t(u8, align, shift);
	} else {
		align = max_t(u8, align, 12);
	}
	temp = vmm->free.rb_node;
	while (temp) {
		struct nvkm_vma *this = rb_entry(temp, typeof(*this), tree);
		if (this->size < size) {
			temp = temp->rb_right;
		} else {
			node = temp;
			temp = temp->rb_left;
		}
	}
	if (unlikely(!node))
		return -ENOSPC;
	do {
		struct nvkm_vma *this = rb_entry(node, typeof(*this), tree);
		struct nvkm_vma *prev = node(this, prev);
		struct nvkm_vma *next = node(this, next);
		const int p = page - vmm->func->page;
		addr = this->addr;
		if (vmm->func->page_block && prev && prev->page != p)
			addr = ALIGN(addr, vmm->func->page_block);
		addr = ALIGN(addr, 1ULL << align);
		tail = this->addr + this->size;
		if (vmm->func->page_block && next && next->page != p)
			tail = ALIGN_DOWN(tail, vmm->func->page_block);
		if (addr <= tail && tail - addr >= size) {
			rb_erase(&this->tree, &vmm->free);
			vma = this;
			break;
		}
	} while ((node = rb_next(node)));
	if (unlikely(!vma))
		return -ENOSPC;
	if (addr != vma->addr) {
		if (!(tmp = nvkm_vma_tail(vma, vma->size + vma->addr - addr))) {
			nvkm_vmm_put_region(vmm, vma);
			return -ENOMEM;
		}
		nvkm_vmm_free_insert(vmm, vma);
		vma = tmp;
	}
	if (size != vma->size) {
		if (!(tmp = nvkm_vma_tail(vma, vma->size - size))) {
			nvkm_vmm_put_region(vmm, vma);
			return -ENOMEM;
		}
		nvkm_vmm_free_insert(vmm, tmp);
	}
	if (sparse && getref)
		ret = nvkm_vmm_ptes_sparse_get(vmm, page, vma->addr, vma->size);
	else if (sparse)
		ret = nvkm_vmm_ptes_sparse(vmm, vma->addr, vma->size, true);
	else if (getref)
		ret = nvkm_vmm_ptes_get(vmm, page, vma->addr, vma->size);
	else
		ret = 0;
	if (ret) {
		nvkm_vmm_put_region(vmm, vma);
		return ret;
	}
	vma->mapref = mapref && !getref;
	vma->sparse = sparse;
	vma->page = page - vmm->func->page;
	vma->refd = getref ? vma->page : NVKM_VMA_PAGE_NONE;
	vma->used = true;
	nvkm_vmm_node_insert(vmm, vma);
	*pvma = vma;
	return 0;
}