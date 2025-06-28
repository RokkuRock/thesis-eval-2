pci_populate_msicap(struct msicap *msicap, int msgnum, int nextptr)
{
	int mmc;
	assert((msgnum & (msgnum - 1)) == 0 && msgnum >= 1 && msgnum <= 32);
	mmc = ffs(msgnum) - 1;
	bzero(msicap, sizeof(struct msicap));
	msicap->capid = PCIY_MSI;
	msicap->nextptr = nextptr;
	msicap->msgctrl = PCIM_MSICTRL_64BIT | (mmc << 1);
}