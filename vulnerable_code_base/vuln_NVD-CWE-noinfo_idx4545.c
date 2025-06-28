static int nested_vmx_check_guest_state(struct kvm_vcpu *vcpu,
					struct vmcs12 *vmcs12,
					enum vm_entry_failure_code *entry_failure_code)
{
	bool ia32e;
	*entry_failure_code = ENTRY_FAIL_DEFAULT;
	if (CC(!nested_guest_cr0_valid(vcpu, vmcs12->guest_cr0)) ||
	    CC(!nested_guest_cr4_valid(vcpu, vmcs12->guest_cr4)))
		return -EINVAL;
	if ((vmcs12->vm_entry_controls & VM_ENTRY_LOAD_DEBUG_CONTROLS) &&
	    CC(!kvm_dr7_valid(vmcs12->guest_dr7)))
		return -EINVAL;
	if ((vmcs12->vm_entry_controls & VM_ENTRY_LOAD_IA32_PAT) &&
	    CC(!kvm_pat_valid(vmcs12->guest_ia32_pat)))
		return -EINVAL;
	if (nested_vmx_check_vmcs_link_ptr(vcpu, vmcs12)) {
		*entry_failure_code = ENTRY_FAIL_VMCS_LINK_PTR;
		return -EINVAL;
	}
	if ((vmcs12->vm_entry_controls & VM_ENTRY_LOAD_IA32_PERF_GLOBAL_CTRL) &&
	    CC(!kvm_valid_perf_global_ctrl(vcpu_to_pmu(vcpu),
					   vmcs12->guest_ia32_perf_global_ctrl)))
		return -EINVAL;
	if (to_vmx(vcpu)->nested.nested_run_pending &&
	    (vmcs12->vm_entry_controls & VM_ENTRY_LOAD_IA32_EFER)) {
		ia32e = (vmcs12->vm_entry_controls & VM_ENTRY_IA32E_MODE) != 0;
		if (CC(!kvm_valid_efer(vcpu, vmcs12->guest_ia32_efer)) ||
		    CC(ia32e != !!(vmcs12->guest_ia32_efer & EFER_LMA)) ||
		    CC(((vmcs12->guest_cr0 & X86_CR0_PG) &&
		     ia32e != !!(vmcs12->guest_ia32_efer & EFER_LME))))
			return -EINVAL;
	}
	if ((vmcs12->vm_entry_controls & VM_ENTRY_LOAD_BNDCFGS) &&
	    (CC(is_noncanonical_address(vmcs12->guest_bndcfgs & PAGE_MASK, vcpu)) ||
	     CC((vmcs12->guest_bndcfgs & MSR_IA32_BNDCFGS_RSVD))))
		return -EINVAL;
	if (nested_check_guest_non_reg_state(vmcs12))
		return -EINVAL;
	return 0;
}