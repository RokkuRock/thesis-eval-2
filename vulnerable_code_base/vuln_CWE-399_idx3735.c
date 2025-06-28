int hugetlb_reserve_pages(struct inode *inode,
					long from, long to,
					struct vm_area_struct *vma,
					vm_flags_t vm_flags)
{
	long ret, chg;
	struct hstate *h = hstate_inode(inode);
	if (vm_flags & VM_NORESERVE)
		return 0;
	if (!vma || vma->vm_flags & VM_MAYSHARE)
		chg = region_chg(&inode->i_mapping->private_list, from, to);
	else {
		struct resv_map *resv_map = resv_map_alloc();
		if (!resv_map)
			return -ENOMEM;
		chg = to - from;
		set_vma_resv_map(vma, resv_map);
		set_vma_resv_flags(vma, HPAGE_RESV_OWNER);
	}
	if (chg < 0)
		return chg;
	if (hugetlb_get_quota(inode->i_mapping, chg))
		return -ENOSPC;
	ret = hugetlb_acct_memory(h, chg);
	if (ret < 0) {
		hugetlb_put_quota(inode->i_mapping, chg);
		return ret;
	}
	if (!vma || vma->vm_flags & VM_MAYSHARE)
		region_add(&inode->i_mapping->private_list, from, to);
	return 0;
}