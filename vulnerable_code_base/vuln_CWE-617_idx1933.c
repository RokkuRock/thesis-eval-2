pci_emul_cmdsts_write(struct pci_vdev *dev, int coff, uint32_t new, int bytes)
{
	int i, rshift;
	uint32_t cmd, cmd2, changed, old, readonly;
	cmd = pci_get_cfgdata16(dev, PCIR_COMMAND);	 
	rshift = (coff & 0x3) * 8;
	readonly = 0xFFFFF880 >> rshift;
	old = CFGREAD(dev, coff, bytes);
	new &= ~readonly;
	new |= (old & readonly);
	CFGWRITE(dev, coff, new, bytes);		 
	cmd2 = pci_get_cfgdata16(dev, PCIR_COMMAND);	 
	changed = cmd ^ cmd2;
	for (i = 0; i <= PCI_BARMAX; i++) {
		switch (dev->bar[i].type) {
		case PCIBAR_NONE:
		case PCIBAR_MEMHI64:
			break;
		case PCIBAR_IO:
			if (changed & PCIM_CMD_PORTEN) {
				if (porten(dev))
					register_bar(dev, i);
				else
					unregister_bar(dev, i);
			}
			break;
		case PCIBAR_MEM32:
		case PCIBAR_MEM64:
			if (changed & PCIM_CMD_MEMEN) {
				if (memen(dev))
					register_bar(dev, i);
				else
					unregister_bar(dev, i);
			}
			break;
		default:
			assert(0);
		}
	}
	pci_lintr_update(dev);
}