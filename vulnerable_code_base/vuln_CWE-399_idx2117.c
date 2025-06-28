int kvm_arch_vcpu_ioctl_run(struct kvm_vcpu *vcpu, struct kvm_run *run)
{
	int ret;
	sigset_t sigsaved;
	if (unlikely(vcpu->arch.target < 0))
		return -ENOEXEC;
	ret = kvm_vcpu_first_run_init(vcpu);
	if (ret)
		return ret;
	if (run->exit_reason == KVM_EXIT_MMIO) {
		ret = kvm_handle_mmio_return(vcpu, vcpu->run);
		if (ret)
			return ret;
	}
	if (vcpu->sigset_active)
		sigprocmask(SIG_SETMASK, &vcpu->sigset, &sigsaved);
	ret = 1;
	run->exit_reason = KVM_EXIT_UNKNOWN;
	while (ret > 0) {
		cond_resched();
		update_vttbr(vcpu->kvm);
		if (vcpu->arch.pause)
			vcpu_pause(vcpu);
		kvm_vgic_flush_hwstate(vcpu);
		kvm_timer_flush_hwstate(vcpu);
		local_irq_disable();
		if (signal_pending(current)) {
			ret = -EINTR;
			run->exit_reason = KVM_EXIT_INTR;
		}
		if (ret <= 0 || need_new_vmid_gen(vcpu->kvm)) {
			local_irq_enable();
			kvm_timer_sync_hwstate(vcpu);
			kvm_vgic_sync_hwstate(vcpu);
			continue;
		}
		trace_kvm_entry(*vcpu_pc(vcpu));
		kvm_guest_enter();
		vcpu->mode = IN_GUEST_MODE;
		ret = kvm_call_hyp(__kvm_vcpu_run, vcpu);
		vcpu->mode = OUTSIDE_GUEST_MODE;
		vcpu->arch.last_pcpu = smp_processor_id();
		kvm_guest_exit();
		trace_kvm_exit(*vcpu_pc(vcpu));
		local_irq_enable();
		kvm_timer_sync_hwstate(vcpu);
		kvm_vgic_sync_hwstate(vcpu);
		ret = handle_exit(vcpu, run, ret);
	}
	if (vcpu->sigset_active)
		sigprocmask(SIG_SETMASK, &sigsaved, NULL);
	return ret;
}