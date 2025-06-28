void fput(struct file *file)
{
	if (atomic_long_dec_and_test(&file->f_count)) {
		struct task_struct *task = current;
		file_sb_list_del(file);
		if (likely(!in_interrupt() && !(task->flags & PF_KTHREAD))) {
			init_task_work(&file->f_u.fu_rcuhead, ____fput);
			if (!task_work_add(task, &file->f_u.fu_rcuhead, true))
				return;
		}
		if (llist_add(&file->f_u.fu_llist, &delayed_fput_list))
			schedule_work(&delayed_fput_work);
	}
}