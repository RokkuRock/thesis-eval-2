static inline int pmd_none_or_trans_huge_or_clear_bad(pmd_t *pmd)
{
	pmd_t pmdval = *pmd;
#ifdef CONFIG_TRANSPARENT_HUGEPAGE
	barrier();
#endif
	if (pmd_none(pmdval))
		return 1;
	if (unlikely(pmd_bad(pmdval))) {
		if (!pmd_trans_huge(pmdval))
			pmd_clear_bad(pmd);
		return 1;
	}
	return 0;
}