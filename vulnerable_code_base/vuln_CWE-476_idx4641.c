static void kvm_vcpu_destroy(struct kvm_vcpu *vcpu)
{
	kvm_dirty_ring_free(&vcpu->dirty_ring);
	kvm_arch_vcpu_destroy(vcpu);
	put_pid(rcu_dereference_protected(vcpu->pid, 1));
	free_page((unsigned long)vcpu->run);
	kmem_cache_free(kvm_vcpu_cache, vcpu);
}