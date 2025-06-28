update_job_run (updateJobPtr job)
{
	if (*(job->request->source) == '|') {
		debug1 (DEBUG_UPDATE, "Recognized local command: %s", job->request->source);
		update_exec_cmd (job);
		return;
	}
	if (strstr (job->request->source, "://") && strncmp (job->request->source, "file://", 7)) {
		network_process_request (job);
		return;
	}
	{
		debug1 (DEBUG_UPDATE, "Recognized file URI: %s", job->request->source);
		update_load_file (job);
		return;
	}
}