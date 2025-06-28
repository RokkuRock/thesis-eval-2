static int perf_trace_event_perm(struct ftrace_event_call *tp_event,
				 struct perf_event *p_event)
{
	if (ftrace_event_is_function(tp_event) &&
	    perf_paranoid_kernel() && !capable(CAP_SYS_ADMIN))
		return -EPERM;
	if (!(p_event->attr.sample_type & PERF_SAMPLE_RAW))
		return 0;
	if (p_event->attach_state == PERF_ATTACH_TASK) {
		if (tp_event->flags & TRACE_EVENT_FL_CAP_ANY)
			return 0;
	}
	if (perf_paranoid_tracepoint_raw() && !capable(CAP_SYS_ADMIN))
		return -EPERM;
	return 0;
}