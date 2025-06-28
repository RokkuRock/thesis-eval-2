PJ_DEF(pj_status_t) pjmedia_vid_conf_add_port( pjmedia_vid_conf *vid_conf,
					       pj_pool_t *parent_pool,
					       pjmedia_port *port,
					       const pj_str_t *name,
					       void *opt,
					       unsigned *p_slot)
{
    pj_pool_t *pool;
    vconf_port *cport;
    unsigned index;
    PJ_ASSERT_RETURN(vid_conf && parent_pool && port, PJ_EINVAL);
    PJ_ASSERT_RETURN(port->info.fmt.type==PJMEDIA_TYPE_VIDEO &&
		     port->info.fmt.detail_type==PJMEDIA_FORMAT_DETAIL_VIDEO,
		     PJ_EINVAL);
    PJ_UNUSED_ARG(opt);
    if (!name)
	name = &port->info.name;
    pj_mutex_lock(vid_conf->mutex);
    if (vid_conf->port_cnt >= vid_conf->opt.max_slot_cnt) {
	pj_assert(!"Too many ports");
	pj_mutex_unlock(vid_conf->mutex);
	return PJ_ETOOMANY;
    }
    for (index=0; index < vid_conf->opt.max_slot_cnt; ++index) {
	if (vid_conf->ports[index] == NULL)
	    break;
    }
    pj_assert(index != vid_conf->opt.max_slot_cnt);
    pool = pj_pool_create(parent_pool->factory, name->ptr, 500, 500, NULL);
    PJ_ASSERT_RETURN(pool, PJ_ENOMEM);
    cport = PJ_POOL_ZALLOC_T(pool, vconf_port);
    PJ_ASSERT_RETURN(cport, PJ_ENOMEM);
    cport->pool = pool;
    cport->port = port;
    cport->format = port->info.fmt;
    cport->idx  = index;
    pj_strdup_with_null(pool, &cport->name, name);
    {
	pjmedia_ratio *fps = &port->info.fmt.det.vid.fps;
	pj_uint32_t vconf_interval = (pj_uint32_t)
				     (TS_CLOCK_RATE * 1.0 /
				     vid_conf->opt.frame_rate);
	cport->ts_interval = (pj_uint32_t)(TS_CLOCK_RATE * 1.0 /
					   fps->num * fps->denum);
	if (cport->ts_interval < vconf_interval) {
	    cport->ts_interval = vconf_interval;
	    PJ_LOG(3,(THIS_FILE, "Warning: frame rate of port %s is higher "
				 "than video conference bridge (%d > %d)",
				 name->ptr, (int)(fps->num/fps->denum),
				 vid_conf->opt.frame_rate));
	}
    }
    {
	const pjmedia_video_format_info *vfi;
	pjmedia_video_apply_fmt_param vafp;
	pj_status_t status;
	vfi = pjmedia_get_video_format_info(NULL, port->info.fmt.id);
	if (!vfi) {
	    PJ_LOG(4,(THIS_FILE, "pjmedia_vid_conf_add_port(): "
				 "unrecognized format %04X",
				 port->info.fmt.id));
	    return PJMEDIA_EBADFMT;
	}
	pj_bzero(&vafp, sizeof(vafp));
	vafp.size = port->info.fmt.det.vid.size;
	status = (*vfi->apply_fmt)(vfi, &vafp);
	if (status != PJ_SUCCESS) {
	    PJ_LOG(4,(THIS_FILE, "pjmedia_vid_conf_add_port(): "
				 "Failed to apply format %04X",
				 port->info.fmt.id));
	    return status;
	}
	if (port->put_frame) {
	    cport->put_buf_size = vafp.framebytes;
	    cport->put_buf = pj_pool_zalloc(cport->pool, cport->put_buf_size);
	}
	if (port->get_frame) {
	    cport->get_buf_size = vafp.framebytes;
	    cport->get_buf = pj_pool_zalloc(cport->pool, cport->get_buf_size);
	}
    }
    cport->listener_slots = (unsigned*)
			    pj_pool_zalloc(pool,
					   vid_conf->opt.max_slot_cnt *
					   sizeof(unsigned));
    PJ_ASSERT_RETURN(cport->listener_slots, PJ_ENOMEM);
    cport->transmitter_slots = (unsigned*)
			       pj_pool_zalloc(pool,
					      vid_conf->opt.max_slot_cnt *
					      sizeof(unsigned));
    PJ_ASSERT_RETURN(cport->transmitter_slots, PJ_ENOMEM);
    cport->render_states = (render_state**)
			   pj_pool_zalloc(pool,
					  vid_conf->opt.max_slot_cnt *
					  sizeof(render_state*));
    PJ_ASSERT_RETURN(cport->render_states, PJ_ENOMEM);
    cport->render_pool = (pj_pool_t**)
			 pj_pool_zalloc(pool,
					vid_conf->opt.max_slot_cnt *
					sizeof(pj_pool_t*));
    PJ_ASSERT_RETURN(cport->render_pool, PJ_ENOMEM);
    vid_conf->ports[index] = cport;
    vid_conf->port_cnt++;
    PJ_LOG(4,(THIS_FILE,"Added port %d (%.*s)",
	      index, (int)cport->name.slen, cport->name.ptr));
    pj_mutex_unlock(vid_conf->mutex);
    if (p_slot) {
	*p_slot = index;
    }
    return PJ_SUCCESS;
}