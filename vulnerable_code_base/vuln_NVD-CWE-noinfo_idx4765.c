static void evtchn_2l_handle_events(unsigned cpu)
{
	int irq;
	xen_ulong_t pending_words;
	xen_ulong_t pending_bits;
	int start_word_idx, start_bit_idx;
	int word_idx, bit_idx;
	int i;
	struct shared_info *s = HYPERVISOR_shared_info;
	struct vcpu_info *vcpu_info = __this_cpu_read(xen_vcpu);
	irq = irq_from_virq(cpu, VIRQ_TIMER);
	if (irq != -1) {
		evtchn_port_t evtchn = evtchn_from_irq(irq);
		word_idx = evtchn / BITS_PER_LONG;
		bit_idx = evtchn % BITS_PER_LONG;
		if (active_evtchns(cpu, s, word_idx) & (1ULL << bit_idx))
			generic_handle_irq(irq);
	}
	pending_words = xchg_xen_ulong(&vcpu_info->evtchn_pending_sel, 0);
	start_word_idx = __this_cpu_read(current_word_idx);
	start_bit_idx = __this_cpu_read(current_bit_idx);
	word_idx = start_word_idx;
	for (i = 0; pending_words != 0; i++) {
		xen_ulong_t words;
		words = MASK_LSBS(pending_words, word_idx);
		if (words == 0) {
			word_idx = 0;
			bit_idx = 0;
			continue;
		}
		word_idx = EVTCHN_FIRST_BIT(words);
		pending_bits = active_evtchns(cpu, s, word_idx);
		bit_idx = 0;  
		if (word_idx == start_word_idx) {
			if (i == 0)
				bit_idx = start_bit_idx;
		}
		do {
			xen_ulong_t bits;
			evtchn_port_t port;
			bits = MASK_LSBS(pending_bits, bit_idx);
			if (bits == 0)
				break;
			bit_idx = EVTCHN_FIRST_BIT(bits);
			port = (word_idx * BITS_PER_EVTCHN_WORD) + bit_idx;
			irq = get_evtchn_to_irq(port);
			if (irq != -1)
				generic_handle_irq(irq);
			bit_idx = (bit_idx + 1) % BITS_PER_EVTCHN_WORD;
			__this_cpu_write(current_word_idx,
					 bit_idx ? word_idx :
					 (word_idx+1) % BITS_PER_EVTCHN_WORD);
			__this_cpu_write(current_bit_idx, bit_idx);
		} while (bit_idx != 0);
		if ((word_idx != start_word_idx) || (i != 0))
			pending_words &= ~(1UL << word_idx);
		word_idx = (word_idx + 1) % BITS_PER_EVTCHN_WORD;
	}
}