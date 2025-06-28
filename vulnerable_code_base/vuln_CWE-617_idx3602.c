init_pci(struct vmctx *ctx)
{
	struct mem_range mr;
	struct pci_vdev_ops *ops;
	struct businfo *bi;
	struct slotinfo *si;
	struct funcinfo *fi;
	size_t lowmem;
	int bus, slot, func;
	int success_cnt = 0;
	int error;
	pci_emul_iobase = PCI_EMUL_IOBASE;
	pci_emul_membase32 = vm_get_lowmem_limit(ctx);
	pci_emul_membase64 = PCI_EMUL_MEMBASE64;
	create_gsi_sharing_groups();
	for (bus = 0; bus < MAXBUSES; bus++) {
		bi = pci_businfo[bus];
		if (bi == NULL)
			continue;
		bi->iobase = pci_emul_iobase;
		bi->membase32 = pci_emul_membase32;
		bi->membase64 = pci_emul_membase64;
		for (slot = 0; slot < MAXSLOTS; slot++) {
			si = &bi->slotinfo[slot];
			for (func = 0; func < MAXFUNCS; func++) {
				fi = &si->si_funcs[func];
				if (fi->fi_name == NULL)
					continue;
				ops = pci_emul_finddev(fi->fi_name);
				assert(ops != NULL);
				pr_notice("pci init %s\r\n", fi->fi_name);
				error = pci_emul_init(ctx, ops, bus, slot,
				    func, fi);
				if (error) {
					pr_err("pci %s init failed\n", fi->fi_name);
					goto pci_emul_init_fail;
				}
				success_cnt++;
			}
		}
		pci_emul_iobase += BUSIO_ROUNDUP;
		pci_emul_iobase = roundup2(pci_emul_iobase, BUSIO_ROUNDUP);
		bi->iolimit = pci_emul_iobase;
		pci_emul_membase32 += BUSMEM_ROUNDUP;
		pci_emul_membase32 = roundup2(pci_emul_membase32,
		    BUSMEM_ROUNDUP);
		bi->memlimit32 = pci_emul_membase32;
		pci_emul_membase64 += BUSMEM_ROUNDUP;
		pci_emul_membase64 = roundup2(pci_emul_membase64,
		    BUSMEM_ROUNDUP);
		bi->memlimit64 = pci_emul_membase64;
	}
	error = check_gsi_sharing_violation();
	if (error < 0)
		goto pci_emul_init_fail;
	for (bus = 0; bus < MAXBUSES; bus++) {
		bi = pci_businfo[bus];
		if (bi == NULL)
			continue;
		for (slot = 0; slot < MAXSLOTS; slot++) {
			si = &bi->slotinfo[slot];
			for (func = 0; func < MAXFUNCS; func++) {
				fi = &si->si_funcs[func];
				if (fi->fi_devi == NULL)
					continue;
				pci_lintr_route(fi->fi_devi);
				ops = fi->fi_devi->dev_ops;
				if (ops && ops->vdev_phys_access)
					ops->vdev_phys_access(ctx,
						fi->fi_devi);
			}
		}
	}
	lpc_pirq_routed();
	lowmem = vm_get_lowmem_size(ctx);
	bzero(&mr, sizeof(struct mem_range));
	mr.name = "PCI hole (32-bit)";
	mr.flags = MEM_F_RW;
	mr.base = lowmem;
	mr.size = (4ULL * 1024 * 1024 * 1024) - lowmem;
	mr.handler = pci_emul_fallback_handler;
	error = register_mem_fallback(&mr);
	assert(error == 0);
	bzero(&mr, sizeof(struct mem_range));
	mr.name = "PCI hole (64-bit)";
	mr.flags = MEM_F_RW;
	mr.base = PCI_EMUL_MEMBASE64;
	mr.size = PCI_EMUL_MEMLIMIT64 - PCI_EMUL_MEMBASE64;
	mr.handler = pci_emul_fallback_handler;
	error = register_mem_fallback(&mr);
	assert(error == 0);
	bzero(&mr, sizeof(struct mem_range));
	mr.name = "PCI ECFG";
	mr.flags = MEM_F_RW;
	mr.base = PCI_EMUL_ECFG_BASE;
	mr.size = PCI_EMUL_ECFG_SIZE;
	mr.handler = pci_emul_ecfg_handler;
	error = register_mem(&mr);
	assert(error == 0);
	return 0;
pci_emul_init_fail:
	for (bus = 0; bus < MAXBUSES && success_cnt > 0; bus++) {
		bi = pci_businfo[bus];
		if (bi == NULL)
			continue;
		for (slot = 0; slot < MAXSLOTS && success_cnt > 0; slot++) {
			si = &bi->slotinfo[slot];
			for (func = 0; func < MAXFUNCS; func++) {
				fi = &si->si_funcs[func];
				if (fi->fi_name == NULL)
					continue;
				if (success_cnt-- <= 0)
					break;
				ops = pci_emul_finddev(fi->fi_name);
				assert(ops != NULL);
				pci_emul_deinit(ctx, ops, bus, slot,
				    func, fi);
			}
		}
	}
	return error;
}