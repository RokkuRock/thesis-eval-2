static void consume_one_event(unsigned cpu,
			      struct evtchn_fifo_control_block *control_block,
			      unsigned priority, unsigned long *ready,
			      bool drop)
{
	struct evtchn_fifo_queue *q = &per_cpu(cpu_queue, cpu);
	uint32_t head;
	evtchn_port_t port;
	event_word_t *word;
	head = q->head[priority];
	if (head == 0) {
		virt_rmb();  
		head = control_block->head[priority];
	}
	port = head;
	word = event_word_from_port(port);
	head = clear_linked(word);
	if (head == 0)
		clear_bit(priority, ready);
	if (evtchn_fifo_is_pending(port) && !evtchn_fifo_is_masked(port)) {
		if (unlikely(drop))
			pr_warn("Dropping pending event for port %u\n", port);
		else
			handle_irq_for_port(port);
	}
	q->head[priority] = head;
}