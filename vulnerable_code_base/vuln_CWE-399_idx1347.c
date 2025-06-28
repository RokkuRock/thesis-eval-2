static void update_exception_bitmap(struct kvm_vcpu *vcpu)
{
	u32 eb;
	eb = (1u << PF_VECTOR) | (1u << UD_VECTOR) | (1u << MC_VECTOR) |
	     (1u << NM_VECTOR) | (1u << DB_VECTOR);
	if ((vcpu->guest_debug &
	     (KVM_GUESTDBG_ENABLE | KVM_GUESTDBG_USE_SW_BP)) ==
	    (KVM_GUESTDBG_ENABLE | KVM_GUESTDBG_USE_SW_BP))
		eb |= 1u << BP_VECTOR;
	if (to_vmx(vcpu)->rmode.vm86_active)
		eb = ~0;
	if (enable_ept)
		eb &= ~(1u << PF_VECTOR);  
	if (vcpu->fpu_active)
		eb &= ~(1u << NM_VECTOR);
	if (is_guest_mode(vcpu))
		eb |= get_vmcs12(vcpu)->exception_bitmap;
	vmcs_write32(EXCEPTION_BITMAP, eb);
}