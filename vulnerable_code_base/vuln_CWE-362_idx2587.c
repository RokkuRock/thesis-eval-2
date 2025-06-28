void jbd2_journal_lock_updates(journal_t *journal)
{
	DEFINE_WAIT(wait);
	jbd2_might_wait_for_commit(journal);
	write_lock(&journal->j_state_lock);
	++journal->j_barrier_count;
	if (atomic_read(&journal->j_reserved_credits)) {
		write_unlock(&journal->j_state_lock);
		wait_event(journal->j_wait_reserved,
			   atomic_read(&journal->j_reserved_credits) == 0);
		write_lock(&journal->j_state_lock);
	}
	jbd2_journal_wait_updates(journal);
	write_unlock(&journal->j_state_lock);
	mutex_lock(&journal->j_barrier);
}