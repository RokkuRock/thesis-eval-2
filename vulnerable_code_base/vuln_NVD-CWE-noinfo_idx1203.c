asmlinkage __visible void __init xen_start_kernel(void)
{
	struct physdev_set_iopl set_iopl;
	unsigned long initrd_start = 0;
	int rc;
	if (!xen_start_info)
		return;
	__text_gen_insn(&early_xen_iret_patch,
			JMP32_INSN_OPCODE, &early_xen_iret_patch, &xen_iret,
			JMP32_INSN_SIZE);
	xen_domain_type = XEN_PV_DOMAIN;
	xen_start_flags = xen_start_info->flags;
	xen_setup_features();
	pv_info = xen_info;
	pv_ops.cpu = xen_cpu_ops.cpu;
	xen_init_irq_ops();
	xen_vcpu_info_reset(0);
	x86_platform.get_nmi_reason = xen_get_nmi_reason;
	x86_init.resources.memory_setup = xen_memory_setup;
	x86_init.irqs.intr_mode_select	= x86_init_noop;
	x86_init.irqs.intr_mode_init	= x86_init_noop;
	x86_init.oem.arch_setup = xen_arch_setup;
	x86_init.oem.banner = xen_banner;
	x86_init.hyper.init_platform = xen_pv_init_platform;
	x86_init.hyper.guest_late_init = xen_pv_guest_late_init;
	xen_setup_machphys_mapping();
	xen_init_mmu_ops();
	__supported_pte_mask &= ~_PAGE_GLOBAL;
	__default_kernel_pte_mask &= ~_PAGE_GLOBAL;
	xen_build_dynamic_phys_to_machine();
	get_cpu_cap(&boot_cpu_data);
	x86_configure_nx();
	xen_setup_gdt(0);
	get_cpu_address_sizes(&boot_cpu_data);
	per_cpu(xen_vcpu_id, 0) = 0;
	idt_setup_early_handler();
	xen_init_capabilities();
#ifdef CONFIG_X86_LOCAL_APIC
	xen_init_apic();
#endif
	machine_ops = xen_machine_ops;
	xen_initial_gdt = &per_cpu(gdt_page, 0);
	xen_smp_init();
#ifdef CONFIG_ACPI_NUMA
	disable_srat();
#endif
	WARN_ON(xen_cpuhp_setup(xen_cpu_up_prepare_pv, xen_cpu_dead_pv));
	local_irq_disable();
	early_boot_irqs_disabled = true;
	xen_raw_console_write("mapping kernel into physical memory\n");
	xen_setup_kernel_pagetable((pgd_t *)xen_start_info->pt_base,
				   xen_start_info->nr_pages);
	xen_reserve_special_pages();
	set_iopl.iopl = 1;
	rc = HYPERVISOR_physdev_op(PHYSDEVOP_set_iopl, &set_iopl);
	if (rc != 0)
		xen_raw_printk("physdev_op failed %d\n", rc);
	if (xen_start_info->mod_start) {
	    if (xen_start_info->flags & SIF_MOD_START_PFN)
		initrd_start = PFN_PHYS(xen_start_info->mod_start);
	    else
		initrd_start = __pa(xen_start_info->mod_start);
	}
	boot_params.hdr.type_of_loader = (9 << 4) | 0;
	boot_params.hdr.ramdisk_image = initrd_start;
	boot_params.hdr.ramdisk_size = xen_start_info->mod_len;
	boot_params.hdr.cmd_line_ptr = __pa(xen_start_info->cmd_line);
	boot_params.hdr.hardware_subarch = X86_SUBARCH_XEN;
	if (!xen_initial_domain()) {
		if (pci_xen)
			x86_init.pci.arch_init = pci_xen_init;
		x86_platform.set_legacy_features =
				xen_domu_set_legacy_features;
	} else {
		const struct dom0_vga_console_info *info =
			(void *)((char *)xen_start_info +
				 xen_start_info->console.dom0.info_off);
		struct xen_platform_op op = {
			.cmd = XENPF_firmware_info,
			.interface_version = XENPF_INTERFACE_VERSION,
			.u.firmware_info.type = XEN_FW_KBD_SHIFT_FLAGS,
		};
		x86_platform.set_legacy_features =
				xen_dom0_set_legacy_features;
		xen_init_vga(info, xen_start_info->console.dom0.info_size);
		xen_start_info->console.domU.mfn = 0;
		xen_start_info->console.domU.evtchn = 0;
		if (HYPERVISOR_platform_op(&op) == 0)
			boot_params.kbd_status = op.u.firmware_info.u.kbd_shift_flags;
		pci_request_acs();
		xen_acpi_sleep_register();
		xen_boot_params_init_edd();
#ifdef CONFIG_ACPI
		acpi_disable_cmcff = 1;
#endif
	}
	xen_add_preferred_consoles();
#ifdef CONFIG_PCI
	pci_probe &= ~PCI_PROBE_BIOS;
#endif
	xen_raw_console_write("about to get started...\n");
	xen_setup_runstate_info(0);
	xen_efi_init(&boot_params);
	cr4_init_shadow();  
	x86_64_start_reservations((char *)__pa_symbol(&boot_params));
}