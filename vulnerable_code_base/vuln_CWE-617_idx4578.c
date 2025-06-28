pci_emul_add_capability(struct pci_vdev *dev, u_char *capdata, int caplen)
{
	int i, capoff, reallen;
	uint16_t sts;
	assert(caplen > 0);
	reallen = roundup2(caplen, 4);		 
	sts = pci_get_cfgdata16(dev, PCIR_STATUS);
	if ((sts & PCIM_STATUS_CAPPRESENT) == 0)
		capoff = CAP_START_OFFSET;
	else
		capoff = dev->capend + 1;
	if (capoff + reallen > PCI_REGMAX + 1)
		return -1;
	if ((sts & PCIM_STATUS_CAPPRESENT) == 0) {
		pci_set_cfgdata8(dev, PCIR_CAP_PTR, capoff);
		pci_set_cfgdata16(dev, PCIR_STATUS, sts|PCIM_STATUS_CAPPRESENT);
	} else
		pci_set_cfgdata8(dev, dev->prevcap + 1, capoff);
	for (i = 0; i < caplen; i++)
		pci_set_cfgdata8(dev, capoff + i, capdata[i]);
	pci_set_cfgdata8(dev, capoff + 1, 0);
	dev->prevcap = capoff;
	dev->capend = capoff + reallen - 1;
	return 0;
}