static ssize_t userfaultfd_ctx_read(struct userfaultfd_ctx *ctx, int no_wait,
				    struct uffd_msg *msg)
{
	ssize_t ret;
	DECLARE_WAITQUEUE(wait, current);
	struct userfaultfd_wait_queue *uwq;
	LIST_HEAD(fork_event);
	struct userfaultfd_ctx *fork_nctx = NULL;
	spin_lock(&ctx->fd_wqh.lock);
	__add_wait_queue(&ctx->fd_wqh, &wait);
	for (;;) {
		set_current_state(TASK_INTERRUPTIBLE);
		spin_lock(&ctx->fault_pending_wqh.lock);
		uwq = find_userfault(ctx);
		if (uwq) {
			write_seqcount_begin(&ctx->refile_seq);
			list_del(&uwq->wq.entry);
			__add_wait_queue(&ctx->fault_wqh, &uwq->wq);
			write_seqcount_end(&ctx->refile_seq);
			*msg = uwq->msg;
			spin_unlock(&ctx->fault_pending_wqh.lock);
			ret = 0;
			break;
		}
		spin_unlock(&ctx->fault_pending_wqh.lock);
		spin_lock(&ctx->event_wqh.lock);
		uwq = find_userfault_evt(ctx);
		if (uwq) {
			*msg = uwq->msg;
			if (uwq->msg.event == UFFD_EVENT_FORK) {
				fork_nctx = (struct userfaultfd_ctx *)
					(unsigned long)
					uwq->msg.arg.reserved.reserved1;
				list_move(&uwq->wq.entry, &fork_event);
				spin_unlock(&ctx->event_wqh.lock);
				ret = 0;
				break;
			}
			userfaultfd_event_complete(ctx, uwq);
			spin_unlock(&ctx->event_wqh.lock);
			ret = 0;
			break;
		}
		spin_unlock(&ctx->event_wqh.lock);
		if (signal_pending(current)) {
			ret = -ERESTARTSYS;
			break;
		}
		if (no_wait) {
			ret = -EAGAIN;
			break;
		}
		spin_unlock(&ctx->fd_wqh.lock);
		schedule();
		spin_lock(&ctx->fd_wqh.lock);
	}
	__remove_wait_queue(&ctx->fd_wqh, &wait);
	__set_current_state(TASK_RUNNING);
	spin_unlock(&ctx->fd_wqh.lock);
	if (!ret && msg->event == UFFD_EVENT_FORK) {
		ret = resolve_userfault_fork(ctx, fork_nctx, msg);
		if (!ret) {
			spin_lock(&ctx->event_wqh.lock);
			if (!list_empty(&fork_event)) {
				uwq = list_first_entry(&fork_event,
						       typeof(*uwq),
						       wq.entry);
				list_del(&uwq->wq.entry);
				__add_wait_queue(&ctx->event_wqh, &uwq->wq);
				userfaultfd_event_complete(ctx, uwq);
			}
			spin_unlock(&ctx->event_wqh.lock);
		}
	}
	return ret;
}