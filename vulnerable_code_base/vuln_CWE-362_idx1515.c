static bool blk_kick_flush(struct request_queue *q, struct blk_flush_queue *fq)
{
	struct list_head *pending = &fq->flush_queue[fq->flush_pending_idx];
	struct request *first_rq =
		list_first_entry(pending, struct request, flush.list);
	struct request *flush_rq = fq->flush_rq;
	if (fq->flush_pending_idx != fq->flush_running_idx || list_empty(pending))
		return false;
	if (!list_empty(&fq->flush_data_in_flight) &&
	    time_before(jiffies,
			fq->flush_pending_since + FLUSH_PENDING_TIMEOUT))
		return false;
	fq->flush_pending_idx ^= 1;
	blk_rq_init(q, flush_rq);
	if (q->mq_ops) {
		flush_rq->mq_ctx = first_rq->mq_ctx;
		flush_rq->tag = first_rq->tag;
	}
	flush_rq->cmd_type = REQ_TYPE_FS;
	flush_rq->cmd_flags = WRITE_FLUSH | REQ_FLUSH_SEQ;
	flush_rq->rq_disk = first_rq->rq_disk;
	flush_rq->end_io = flush_end_io;
	return blk_flush_queue_rq(flush_rq, false);
}