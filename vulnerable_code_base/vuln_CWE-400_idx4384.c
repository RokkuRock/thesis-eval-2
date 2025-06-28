static GF_Err gf_filter_pid_configure(GF_Filter *filter, GF_FilterPid *pid, GF_PidConnectType ctype)
{
	u32 i, count;
	GF_Err e;
	Bool refire_events=GF_FALSE;
	Bool new_pid_inst=GF_FALSE;
	Bool remove_filter=GF_FALSE;
	GF_FilterPidInst *pidinst=NULL;
	GF_Filter *alias_orig = NULL;
	if (filter->multi_sink_target) {
		alias_orig = filter;
		filter = filter->multi_sink_target;
	}
	assert(filter->freg->configure_pid);
	if (filter->finalized) {
		GF_LOG(GF_LOG_ERROR, GF_LOG_FILTER, ("Trying to configure PID %s in finalized filter %s\n",  pid->name, filter->name));
		if (ctype==GF_PID_CONF_CONNECT) {
			assert(pid->filter->out_pid_connection_pending);
			safe_int_dec(&pid->filter->out_pid_connection_pending);
		}
		return GF_SERVICE_ERROR;
	}
	if (filter->detached_pid_inst) {
		count = gf_list_count(filter->detached_pid_inst);
		for (i=0; i<count; i++) {
			pidinst = gf_list_get(filter->detached_pid_inst, i);
			if (pidinst->filter==filter) {
				gf_list_rem(filter->detached_pid_inst, i);
				pidinst->filter = filter;
				pidinst->pid = pid;
				assert(!pidinst->props);
				if (ctype == GF_PID_CONF_CONNECT) {
					new_pid_inst=GF_TRUE;
					if (!pid->filter->nb_pids_playing && (pidinst->is_playing || pidinst->is_paused))
						refire_events = GF_TRUE;
				}
				assert(pidinst->detach_pending);
				safe_int_dec(&pidinst->detach_pending);
				if (filter->sticky == 2)
					filter->sticky = 0;
				break;
			}
			pidinst=NULL;
		}
		if (! gf_list_count(filter->detached_pid_inst)) {
			gf_list_del(filter->detached_pid_inst);
			filter->detached_pid_inst = NULL;
		}
	}
	if (!pidinst) {
		count = pid->num_destinations;
		for (i=0; i<count; i++) {
			pidinst = gf_list_get(pid->destinations, i);
			if (pidinst->filter==filter) {
				break;
			}
			pidinst=NULL;
		}
	}
	if (!pidinst) {
		if (ctype != GF_PID_CONF_CONNECT) {
			GF_LOG(GF_LOG_ERROR, GF_LOG_FILTER, ("Trying to disconnect PID %s not present in filter %s inputs\n",  pid->name, filter->name));
			return GF_SERVICE_ERROR;
		}
		pidinst = gf_filter_pid_inst_new(filter, pid);
		new_pid_inst=GF_TRUE;
	}
	if (!pidinst->alias_orig)
		pidinst->alias_orig = alias_orig;
	if (new_pid_inst) {
		assert(pidinst);
		gf_mx_p(pid->filter->tasks_mx);
		GF_LOG(GF_LOG_DEBUG, GF_LOG_FILTER, ("Registering %s:%s as destination for %s:%s\n", pid->filter->name, pid->name, pidinst->filter->name, pidinst->pid->name));
		gf_list_add(pid->destinations, pidinst);
		pid->num_destinations = gf_list_count(pid->destinations);
		gf_mx_v(pid->filter->tasks_mx);
		gf_mx_p(filter->tasks_mx);
		if (!filter->input_pids) filter->input_pids = gf_list_new();
		gf_list_add(filter->input_pids, pidinst);
		filter->num_input_pids = gf_list_count(filter->input_pids);
		if (filter->num_input_pids==1) {
			filter->single_source = pidinst->pid->filter;
		} else if (filter->single_source != pidinst->pid->filter) {
			filter->single_source = NULL;
		}
		gf_mx_v(filter->tasks_mx);
		gf_filter_pid_update_caps(pid);
	}
	if (filter->swap_pending) {
		gf_filter_pid_inst_swap(filter, pidinst);
		filter->swap_pending = GF_FALSE;
	}
	filter->in_connect_err = GF_EOS;
#if 0
	FSESS_CHECK_THREAD(filter)
#endif
	GF_LOG(GF_LOG_DEBUG, GF_LOG_FILTER, ("Filter %s PID %s reconfigure\n", pidinst->filter->name, pidinst->pid->name));
	e = filter->freg->configure_pid(filter, (GF_FilterPid*) pidinst, (ctype==GF_PID_CONF_REMOVE) ? GF_TRUE : GF_FALSE);
#ifdef GPAC_MEMORY_TRACKING
	if (filter->session->check_allocs) {
		if (filter->nb_consecutive_process >= filter->max_nb_consecutive_process) {
			filter->max_nb_consecutive_process = filter->nb_consecutive_process;
			filter->max_nb_process = filter->nb_process_since_reset;
			filter->max_stats_nb_alloc = filter->stats_nb_alloc;
			filter->max_stats_nb_calloc = filter->stats_nb_calloc;
			filter->max_stats_nb_realloc = filter->stats_nb_realloc;
			filter->max_stats_nb_free = filter->stats_nb_free;
		}
		filter->stats_mem_allocated = 0;
		filter->stats_nb_alloc = filter->stats_nb_realloc = filter->stats_nb_free = 0;
		filter->nb_process_since_reset = filter->nb_consecutive_process = 0;
	}
#endif
	if ((e==GF_OK) && (filter->in_connect_err<GF_OK))
		e = filter->in_connect_err;
	filter->in_connect_err = GF_OK;
	if (e==GF_OK) {
		if (new_pid_inst) {
			GF_LOG(GF_LOG_INFO, GF_LOG_FILTER, ("Filter %s (%p) PID %s (%p) (%d fan-out) connected to filter %s (%p)\n", pid->filter->name, pid->filter, pid->name, pid, pid->num_destinations, filter->name, filter));
		}
		gf_list_reset(pidinst->pid->filter->blacklisted);
	}
	else if ((ctype==GF_PID_CONF_RECONFIG) && (e != GF_FILTER_NOT_SUPPORTED)) {
		pidinst->is_end_of_stream = GF_TRUE;
		if (e==GF_BAD_PARAM) {
			GF_LOG(GF_LOG_ERROR, GF_LOG_FILTER, ("Failed to reconfigure PID %s:%s in filter %s: %s\n", pid->filter->name, pid->name, filter->name, gf_error_to_string(e) ));
			filter->session->last_connect_error = e;
		} else {
			GF_LOG(GF_LOG_INFO, GF_LOG_FILTER, ("Failed to reconfigure PID %s:%s in filter %s: %s, reloading filter graph\n", pid->filter->name, pid->name, filter->name, gf_error_to_string(e) ));
			gf_list_add(pid->filter->blacklisted, (void *) filter->freg);
			gf_filter_relink_dst(pidinst, e);
		}
	} else {
		gf_mx_p(filter->tasks_mx);
		gf_list_del_item(filter->input_pids, pidinst);
		filter->num_input_pids = gf_list_count(filter->input_pids);
		if (!filter->num_input_pids)
			filter->single_source = NULL;
		filter->freg->configure_pid(filter, (GF_FilterPid *) pidinst, GF_TRUE);
		gf_mx_v(filter->tasks_mx);
		gf_mx_p(pidinst->pid->filter->tasks_mx);
		gf_list_del_item(pidinst->pid->destinations, pidinst);
		pidinst->pid->num_destinations = gf_list_count(pidinst->pid->destinations);
		gf_filter_instance_detach_pid(pidinst);
		gf_mx_v(pidinst->pid->filter->tasks_mx);
		if (new_pid_inst) {
			gf_mx_p(pid->filter->tasks_mx);
			gf_list_del_item(pid->destinations, pidinst);
			pid->num_destinations = gf_list_count(pid->destinations);
			gf_mx_p(pid->filter->tasks_mx);
			count = gf_fq_count(pid->filter->tasks);
			for (i=0; i<count; i++) {
				GF_FSTask *t = gf_fq_get(pid->filter->tasks, i);
				if (t->pid == (GF_FilterPid *) pidinst) {
					t->run_task = task_canceled;
				}
			}
			gf_mx_v(pid->filter->tasks_mx);
			gf_filter_pid_inst_del(pidinst);
			gf_mx_v(pid->filter->tasks_mx);
		}
		if (e==GF_REQUIRES_NEW_INSTANCE) {
			GF_Filter *new_filter = gf_filter_clone(filter, pid->filter);
			if (new_filter) {
				GF_LOG(GF_LOG_DEBUG, GF_LOG_FILTER, ("Clone filter %s, new instance for pid %s\n", filter->name, pid->name));
				gf_filter_pid_post_connect_task(new_filter, pid);
				return GF_OK;
			} else {
				GF_LOG(GF_LOG_ERROR, GF_LOG_FILTER, ("Failed to clone filter %s\n", filter->name));
				e = GF_OUT_OF_MEM;
			}
		}
		if (e && (ctype==GF_PID_CONF_REMOVE)) {
			GF_LOG(GF_LOG_ERROR, GF_LOG_FILTER, ("Failed to disconnect filter %s PID %s from filter %s: %s\n", pid->filter->name, pid->name, filter->name, gf_error_to_string(e) ));
		}
		else if (e) {
			if (e!= GF_EOS) {
				GF_LOG(GF_LOG_ERROR, GF_LOG_FILTER, ("Failed to connect filter %s PID %s to filter %s: %s\n", pid->filter->name, pid->name, filter->name, gf_error_to_string(e) ));
			}
			if ((e==GF_BAD_PARAM)
				|| (e==GF_SERVICE_ERROR)
				|| (e==GF_REMOTE_SERVICE_ERROR)
				|| (e==GF_FILTER_NOT_SUPPORTED)
				|| (e==GF_EOS)
				|| (filter->session->flags & GF_FS_FLAG_NO_REASSIGN)
			) {
				if (filter->session->flags & GF_FS_FLAG_NO_REASSIGN) {
					GF_LOG(GF_LOG_ERROR, GF_LOG_FILTER, ("Filter reassignment disabled, skippping chain reload for filter %s PID %s\n", pid->filter->name, pid->name ));
				}
				if (e!= GF_EOS) {
					filter->session->last_connect_error = e;
				}
				if (ctype==GF_PID_CONF_CONNECT) {
					GF_FilterEvent evt;
					GF_FEVT_INIT(evt, GF_FEVT_PLAY, pid);
					gf_filter_pid_send_event_internal(pid, &evt, GF_TRUE);
					GF_FEVT_INIT(evt, GF_FEVT_STOP, pid);
					gf_filter_pid_send_event_internal(pid, &evt, GF_TRUE);
					gf_filter_pid_set_eos(pid);
					if (pid->filter->freg->process_event) {
						GF_FEVT_INIT(evt, GF_FEVT_CONNECT_FAIL, pid);
						gf_filter_pid_send_event_internal(pid, &evt, GF_TRUE);
					}
					if (!filter->num_input_pids && !filter->num_output_pids) {
						remove_filter = GF_TRUE;
					}
				}
			} else if (filter->has_out_caps) {
				Bool unload_filter = GF_TRUE;
				GF_LOG(GF_LOG_WARNING, GF_LOG_FILTER, ("Blacklisting %s as output from %s and retrying connections\n", filter->name, pid->filter->name));
				gf_list_add(pid->filter->blacklisted, (void *) filter->freg);
				gf_mx_p(filter->tasks_mx);
				while (gf_list_count(filter->input_pids)) {
					GF_FilterPidInst *a_pidinst = gf_list_pop_back(filter->input_pids);
					FSESS_CHECK_THREAD(filter)
					filter->num_input_pids--;
					filter->freg->configure_pid(filter, (GF_FilterPid *) a_pidinst, GF_TRUE);
					gf_filter_pid_post_init_task(a_pidinst->pid->filter, a_pidinst->pid);
					gf_fs_post_task(filter->session, gf_filter_pid_inst_delete_task, a_pidinst->pid->filter, a_pidinst->pid, "pid_inst_delete", a_pidinst);
					unload_filter = GF_FALSE;
				}
				filter->num_input_pids = 0;
				filter->single_source = NULL;
				filter->removed = 1;
				filter->has_pending_pids = GF_FALSE;
				gf_mx_v(filter->tasks_mx);
				if (ctype==GF_PID_CONF_CONNECT) {
					assert(pid->filter->out_pid_connection_pending);
					safe_int_dec(&pid->filter->out_pid_connection_pending);
				}
				gf_filter_pid_post_init_task(pid->filter, pid);
				if (unload_filter) {
					assert(!gf_list_count(filter->input_pids));
					if (filter->num_output_pids) {
						for (i=0; i<filter->num_output_pids; i++) {
							u32 j;
							GF_FilterPid *opid = gf_list_get(filter->output_pids, i);
							for (j=0; j< opid->num_destinations; j++) {
								GF_FilterPidInst *a_pidi = gf_list_get(opid->destinations, j);
								a_pidi->pid = NULL;
							}
							gf_list_reset(opid->destinations);
							opid->num_destinations = 0;
							gf_filter_pid_remove(opid);
						}
					}
					filter->swap_pidinst_src = NULL;
					if (filter->swap_pidinst_dst) {
						GF_Filter *target = filter->swap_pidinst_dst->filter;
						assert(target);
						if (!target->detached_pid_inst) {
							target->detached_pid_inst = gf_list_new();
						}
						if (filter->swap_pidinst_dst->props) {
							filter->swap_pidinst_dst->props = NULL;
						}
						filter->swap_pidinst_dst->pid = NULL;
						if (gf_list_find(target->detached_pid_inst, filter->swap_pidinst_dst)<0)
							gf_list_add(target->detached_pid_inst, filter->swap_pidinst_dst);
					}
					filter->swap_pidinst_dst = NULL;
					if (filter->on_setup_error) {
						gf_filter_notification_failure(filter, e, GF_TRUE);
					} else {
						gf_filter_post_remove(filter);
					}
				}
				return e;
			} else {
				GF_LOG(GF_LOG_ERROR, GF_LOG_FILTER, ("Failed to reconfigure input of sink %s, cannot rebuild graph\n", filter->name));
				if (pid->filter->freg->process_event) {
					GF_FilterEvent evt;
					GF_FEVT_INIT(evt, GF_FEVT_CONNECT_FAIL, pid);
					pid->filter->freg->process_event(pid->filter, &evt);
				}
				filter->session->last_connect_error = e;
			}
		} else {
			filter->session->last_connect_error = GF_OK;
		}
		if (filter->session->requires_solved_graph)
			return e;
	}
	if (filter->has_pending_pids) {
		filter->has_pending_pids = GF_FALSE;
		while (gf_fq_count(filter->pending_pids)) {
			GF_FilterPid *a_pid=gf_fq_pop(filter->pending_pids);
			if (pid->is_playing && filter->is_pid_adaptation_filter)
				a_pid->is_playing = GF_TRUE;
			gf_filter_pid_post_init_task(filter, a_pid);
		}
	}
	if (ctype==GF_PID_CONF_REMOVE) {
		gf_mx_p(filter->tasks_mx);
		gf_list_del_item(filter->input_pids, pidinst);
		filter->num_input_pids = gf_list_count(filter->input_pids);
		if (!filter->num_input_pids)
			filter->single_source = NULL;
		gf_mx_v(filter->tasks_mx);
		gf_mx_p(pidinst->pid->filter->tasks_mx);
		pidinst->pid->num_pidinst_del_pending ++;
		gf_list_del_item(pidinst->pid->destinations, pidinst);
		pidinst->pid->num_destinations = gf_list_count(pidinst->pid->destinations);
		gf_filter_instance_detach_pid(pidinst);
		gf_mx_v(pidinst->pid->filter->tasks_mx);
		if (!filter->num_input_pids && !filter->sticky) {
			gf_filter_reset_pending_packets(filter);
			filter->removed = 1;
		}
		gf_fs_post_task(filter->session, gf_filter_pid_inst_delete_task, pid->filter, pid, "pid_inst_delete", pidinst);
		return e;
	}
	if (ctype==GF_PID_CONF_CONNECT) {
		assert(pid->filter->out_pid_connection_pending);
		if (safe_int_dec(&pid->filter->out_pid_connection_pending) == 0) {
			if (refire_events) {
				GF_FilterEvent evt;
				if (pidinst->is_playing) {
					pidinst->is_playing = GF_FALSE;
					GF_FEVT_INIT(evt, GF_FEVT_PLAY, (GF_FilterPid*)pidinst);
					gf_filter_pid_send_event((GF_FilterPid *)pidinst, &evt);
				}
				if (pidinst->is_paused) {
					pidinst->is_paused = GF_FALSE;
					GF_FEVT_INIT(evt, GF_FEVT_PAUSE, (GF_FilterPid*)pidinst);
					gf_filter_pid_send_event((GF_FilterPid *)pidinst, &evt);
				}
			}
			if (e==GF_OK) {
				if (pid->filter->postponed_packets || pid->filter->pending_packets || pid->filter->nb_caps_renegociate) {
					gf_filter_post_process_task(pid->filter);
				}
			}
		}
		if (remove_filter && !filter->sticky)
			gf_filter_post_remove(filter);
	}
	gf_filter_pid_update_caps(pid);
	return e;