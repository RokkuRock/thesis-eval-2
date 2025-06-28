nvkm_vmm_put_locked(struct nvkm_vmm *vmm, struct nvkm_vma *vma)
{
	const struct nvkm_vmm_page *page = vmm->func->page;
	struct nvkm_vma *next = vma;
	BUG_ON(vma->part);
	if (vma->mapref || !vma->sparse) {
		do {
			const bool map = next->memory != NULL;
			const u8  refd = next->refd;
			const u64 addr = next->addr;
			u64 size = next->size;
			while ((next = node(next, next)) && next->part &&
			       (next->memory != NULL) == map &&
			       (next->refd == refd))
				size += next->size;
			if (map) {
				nvkm_vmm_ptes_unmap_put(vmm, &page[refd], addr,
							size, vma->sparse);
			} else
			if (refd != NVKM_VMA_PAGE_NONE) {
				nvkm_vmm_ptes_put(vmm, &page[refd], addr, size);
			}
		} while (next && next->part);
	}
	next = vma;
	do {
		if (next->memory)
			nvkm_vmm_unmap_region(vmm, next);
	} while ((next = node(vma, next)) && next->part);
	if (vma->sparse && !vma->mapref) {
		nvkm_vmm_ptes_sparse_put(vmm, &page[vma->refd], vma->addr, vma->size);
	} else
	if (vma->sparse) {
		nvkm_vmm_ptes_sparse(vmm, vma->addr, vma->size, false);
	}
	rb_erase(&vma->tree, &vmm->root);
	vma->page = NVKM_VMA_PAGE_NONE;
	vma->refd = NVKM_VMA_PAGE_NONE;
	vma->used = false;
	vma->user = false;
	nvkm_vmm_put_region(vmm, vma);
}