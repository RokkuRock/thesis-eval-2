int ring_buffer_resize(struct ring_buffer *buffer, unsigned long size,
			int cpu_id)
{
	struct ring_buffer_per_cpu *cpu_buffer;
	unsigned long nr_pages;
	int cpu, err = 0;
	if (!buffer)
		return size;
	if (cpu_id != RING_BUFFER_ALL_CPUS &&
	    !cpumask_test_cpu(cpu_id, buffer->cpumask))
		return size;
	size = DIV_ROUND_UP(size, BUF_PAGE_SIZE);
	size *= BUF_PAGE_SIZE;
	if (size < BUF_PAGE_SIZE * 2)
		size = BUF_PAGE_SIZE * 2;
	nr_pages = DIV_ROUND_UP(size, BUF_PAGE_SIZE);
	if (atomic_read(&buffer->resize_disabled))
		return -EBUSY;
	mutex_lock(&buffer->mutex);
	if (cpu_id == RING_BUFFER_ALL_CPUS) {
		for_each_buffer_cpu(buffer, cpu) {
			cpu_buffer = buffer->buffers[cpu];
			cpu_buffer->nr_pages_to_update = nr_pages -
							cpu_buffer->nr_pages;
			if (cpu_buffer->nr_pages_to_update <= 0)
				continue;
			INIT_LIST_HEAD(&cpu_buffer->new_pages);
			if (__rb_allocate_pages(cpu_buffer->nr_pages_to_update,
						&cpu_buffer->new_pages, cpu)) {
				err = -ENOMEM;
				goto out_err;
			}
		}
		get_online_cpus();
		for_each_buffer_cpu(buffer, cpu) {
			cpu_buffer = buffer->buffers[cpu];
			if (!cpu_buffer->nr_pages_to_update)
				continue;
			if (!cpu_online(cpu)) {
				rb_update_pages(cpu_buffer);
				cpu_buffer->nr_pages_to_update = 0;
			} else {
				schedule_work_on(cpu,
						&cpu_buffer->update_pages_work);
			}
		}
		for_each_buffer_cpu(buffer, cpu) {
			cpu_buffer = buffer->buffers[cpu];
			if (!cpu_buffer->nr_pages_to_update)
				continue;
			if (cpu_online(cpu))
				wait_for_completion(&cpu_buffer->update_done);
			cpu_buffer->nr_pages_to_update = 0;
		}
		put_online_cpus();
	} else {
		if (!cpumask_test_cpu(cpu_id, buffer->cpumask))
			goto out;
		cpu_buffer = buffer->buffers[cpu_id];
		if (nr_pages == cpu_buffer->nr_pages)
			goto out;
		cpu_buffer->nr_pages_to_update = nr_pages -
						cpu_buffer->nr_pages;
		INIT_LIST_HEAD(&cpu_buffer->new_pages);
		if (cpu_buffer->nr_pages_to_update > 0 &&
			__rb_allocate_pages(cpu_buffer->nr_pages_to_update,
					    &cpu_buffer->new_pages, cpu_id)) {
			err = -ENOMEM;
			goto out_err;
		}
		get_online_cpus();
		if (!cpu_online(cpu_id))
			rb_update_pages(cpu_buffer);
		else {
			schedule_work_on(cpu_id,
					 &cpu_buffer->update_pages_work);
			wait_for_completion(&cpu_buffer->update_done);
		}
		cpu_buffer->nr_pages_to_update = 0;
		put_online_cpus();
	}
 out:
	if (atomic_read(&buffer->record_disabled)) {
		atomic_inc(&buffer->record_disabled);
		synchronize_sched();
		for_each_buffer_cpu(buffer, cpu) {
			cpu_buffer = buffer->buffers[cpu];
			rb_check_pages(cpu_buffer);
		}
		atomic_dec(&buffer->record_disabled);
	}
	mutex_unlock(&buffer->mutex);
	return size;
 out_err:
	for_each_buffer_cpu(buffer, cpu) {
		struct buffer_page *bpage, *tmp;
		cpu_buffer = buffer->buffers[cpu];
		cpu_buffer->nr_pages_to_update = 0;
		if (list_empty(&cpu_buffer->new_pages))
			continue;
		list_for_each_entry_safe(bpage, tmp, &cpu_buffer->new_pages,
					list) {
			list_del_init(&bpage->list);
			free_buffer_page(bpage);
		}
	}
	mutex_unlock(&buffer->mutex);
	return err;
}