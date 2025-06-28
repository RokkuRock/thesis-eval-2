tport_t *tport_tsend(tport_t *self,
		     msg_t *msg,
		     tp_name_t const *_tpn,
		     tag_type_t tag, tag_value_t value, ...)
{
  ta_list ta;
  tagi_t const *t;
  int reuse, sdwn_after, close_after, resolved = 0, fresh;
  unsigned mtu;
  su_addrinfo_t *ai;
  tport_primary_t *primary;
  tp_name_t tpn[1];
  struct sigcomp_compartment *cc;
  assert(self);
  if (!self || !msg || !_tpn) {
    msg_set_errno(msg, EINVAL);
    return NULL;
  }
  *tpn = *_tpn;
  SU_DEBUG_7(("tport_tsend(%p) tpn = " TPN_FORMAT "\n",
	      (void *)self, TPN_ARGS(tpn)));
  if (tport_is_master(self)) {
    primary = (tport_primary_t *)tport_primary_by_name(self, tpn);
    if (!primary) {
      msg_set_errno(msg, EPROTONOSUPPORT);
      return NULL;
    }
  }
  else {
    primary = self->tp_pri;
  }
  ta_start(ta, tag, value);
  reuse = primary->pri_primary->tp_reusable && self->tp_reusable;
  fresh = 0;
  sdwn_after = 0;
  close_after = 0;
  mtu = 0;
  cc = NULL;
  for (t = ta_args(ta); t; t = tl_next(t)) {
    tag_type_t tt = t->t_tag;
    if (tptag_reuse == tt)
      reuse = t->t_value != 0;
    else if (tptag_mtu == tt)
      mtu = t->t_value;
    else if (tptag_sdwn_after == tt)
      sdwn_after = t->t_value != 0;
    else if (tptag_close_after == tt)
      close_after = t->t_value != 0;
    else if (tptag_fresh == tt)
      fresh = t->t_value != 0;
    else if (tptag_compartment == tt)
      cc = (struct sigcomp_compartment *)t->t_value;
  }
  ta_end(ta);
  fresh = fresh || !reuse;
  ai = msg_addrinfo(msg);
  ai->ai_flags = 0;
  tpn->tpn_comp = tport_canonize_comp(tpn->tpn_comp);
  if (tpn->tpn_comp) {
    ai->ai_flags |= TP_AI_COMPRESSED;
    SU_DEBUG_9(("%s: compressed msg(%p) with %s\n",
		__func__, (void *)msg, tpn->tpn_comp));
  }
  if (!tpn->tpn_comp || cc == NONE)
    cc = NULL;
  if (sdwn_after)
    ai->ai_flags |= TP_AI_SHUTDOWN;
  if (close_after)
    ai->ai_flags |= TP_AI_CLOSE;
  if (fresh) {
    self = primary->pri_primary;
  }
  else if (tport_is_secondary(self) && tport_is_clear_to_send(self)) {
	;
  }
  else {
    if (tport_resolve(primary->pri_primary, msg, tpn) < 0) {
      return NULL;
    }
    resolved = 1;
    tport_t* secondary = NULL ;
    tport_t* tp = tport_primaries( self ) ;
    if (tp) {
      do {
        secondary = tport_by_addrinfo((tport_primary_t *)tp, msg_addrinfo(msg), tpn);
        if (secondary) break;
      } while(NULL != (tp = tport_next(tp)));
    }
    if( secondary ) {
      self = secondary ;
    }
    else {
      self = primary->pri_primary;      
    }
  }
  if (tport_is_primary(self)) {
    if (!resolved && tport_resolve(self, msg, tpn) < 0) {
      return NULL;
    }
    if (tport_is_connection_oriented(self)
	|| self->tp_params->tpp_conn_orient) {
#if 0 && HAVE_UPNP  
      if (upnp_register_upnp_client(1) != 0) {
	upnp_check_for_nat();
      }
#endif
      tpn->tpn_proto = self->tp_protoname;
      if (!cc)
	tpn->tpn_comp = NULL;
      self = tport_connect(primary, msg_addrinfo(msg), tpn);
#if 0 && HAVE_UPNP  
      upnp_deregister_upnp_client(0, 0);
#endif
      if (!self) {
	msg_set_errno(msg, su_errno());
        SU_DEBUG_9(("tport_socket failed in tsend\n" VA_NONE));
	return NULL;
      }
      if (cc)
	tport_sigcomp_assign(self, cc);
    }
  }
  else if (tport_is_secondary(self)) {
    cc = tport_sigcomp_assign_if_needed(self, cc);
  }
  if (cc == NULL)
    tpn->tpn_comp = NULL;
  if (tport_is_secondary(self)) {
    tport_peer_address(self, msg);
    if (sdwn_after || close_after)
      self->tp_reusable = 0;
  }
  if (self->tp_pri->pri_vtable->vtp_prepare
      ? self->tp_pri->pri_vtable->vtp_prepare(self, msg, tpn, cc, mtu) < 0
      : tport_prepare_and_send(self, msg, tpn, cc, mtu) < 0)
    return NULL;
  else
    return self;
}