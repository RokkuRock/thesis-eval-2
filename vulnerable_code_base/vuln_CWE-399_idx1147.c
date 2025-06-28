int hugepage_madvise(struct vm_area_struct *vma,
		     unsigned long *vm_flags, int advice)
{
	switch (advice) {
	case MADV_HUGEPAGE:
		if (*vm_flags & (VM_HUGEPAGE |
				 VM_SHARED   | VM_MAYSHARE   |
				 VM_PFNMAP   | VM_IO      | VM_DONTEXPAND |
				 VM_RESERVED | VM_HUGETLB | VM_INSERTPAGE |
				 VM_MIXEDMAP | VM_SAO))
			return -EINVAL;
		*vm_flags &= ~VM_NOHUGEPAGE;
		*vm_flags |= VM_HUGEPAGE;
		if (unlikely(khugepaged_enter_vma_merge(vma)))
			return -ENOMEM;
		break;
	case MADV_NOHUGEPAGE:
		if (*vm_flags & (VM_NOHUGEPAGE |
				 VM_SHARED   | VM_MAYSHARE   |
				 VM_PFNMAP   | VM_IO      | VM_DONTEXPAND |
				 VM_RESERVED | VM_HUGETLB | VM_INSERTPAGE |
				 VM_MIXEDMAP | VM_SAO))
			return -EINVAL;
		*vm_flags &= ~VM_HUGEPAGE;
		*vm_flags |= VM_NOHUGEPAGE;
		break;
	}
	return 0;
}