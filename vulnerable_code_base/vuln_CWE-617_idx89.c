pci_populate_msixcap(struct msixcap *msixcap, int msgnum, int barnum,
		     uint32_t msix_tab_size)
{
	assert(msix_tab_size % 4096 == 0);
	bzero(msixcap, sizeof(struct msixcap));
	msixcap->capid = PCIY_MSIX;
	msixcap->msgctrl = msgnum - 1;
	msixcap->table_info = barnum & PCIM_MSIX_BIR_MASK;
	msixcap->pba_info = msix_tab_size | (barnum & PCIM_MSIX_BIR_MASK);
}