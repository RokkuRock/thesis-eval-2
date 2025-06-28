static void vmx_set_constant_host_state(struct vcpu_vmx *vmx)
{
	u32 low32, high32;
	unsigned long tmpl;
	struct desc_ptr dt;
	vmcs_writel(HOST_CR0, read_cr0() & ~X86_CR0_TS);   
	vmcs_writel(HOST_CR4, read_cr4());   
	vmcs_writel(HOST_CR3, read_cr3());   
	vmcs_write16(HOST_CS_SELECTOR, __KERNEL_CS);   
#ifdef CONFIG_X86_64
	vmcs_write16(HOST_DS_SELECTOR, 0);
	vmcs_write16(HOST_ES_SELECTOR, 0);
#else
	vmcs_write16(HOST_DS_SELECTOR, __KERNEL_DS);   
	vmcs_write16(HOST_ES_SELECTOR, __KERNEL_DS);   
#endif
	vmcs_write16(HOST_SS_SELECTOR, __KERNEL_DS);   
	vmcs_write16(HOST_TR_SELECTOR, GDT_ENTRY_TSS*8);   
	native_store_idt(&dt);
	vmcs_writel(HOST_IDTR_BASE, dt.address);    
	vmx->host_idt_base = dt.address;
	vmcs_writel(HOST_RIP, vmx_return);  
	rdmsr(MSR_IA32_SYSENTER_CS, low32, high32);
	vmcs_write32(HOST_IA32_SYSENTER_CS, low32);
	rdmsrl(MSR_IA32_SYSENTER_EIP, tmpl);
	vmcs_writel(HOST_IA32_SYSENTER_EIP, tmpl);    
	if (vmcs_config.vmexit_ctrl & VM_EXIT_LOAD_IA32_PAT) {
		rdmsr(MSR_IA32_CR_PAT, low32, high32);
		vmcs_write64(HOST_IA32_PAT, low32 | ((u64) high32 << 32));
	}
}