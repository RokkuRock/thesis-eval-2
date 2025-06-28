bgp_open_receive (struct peer *peer, bgp_size_t size)
{
  int ret;
  u_char version;
  u_char optlen;
  u_int16_t holdtime;
  u_int16_t send_holdtime;
  as_t remote_as;
  struct peer *realpeer;
  struct in_addr remote_id;
  int capability;
  u_int8_t notify_data_remote_as[2];
  u_int8_t notify_data_remote_id[4];
  realpeer = NULL;
  version = stream_getc (peer->ibuf);
  memcpy (notify_data_remote_as, stream_pnt (peer->ibuf), 2);
  remote_as  = stream_getw (peer->ibuf);
  holdtime = stream_getw (peer->ibuf);
  memcpy (notify_data_remote_id, stream_pnt (peer->ibuf), 4);
  remote_id.s_addr = stream_get_ipv4 (peer->ibuf);
  if (BGP_DEBUG (normal, NORMAL))
    zlog_debug ("%s rcv OPEN, version %d, remote-as %d, holdtime %d, id %s",
	       peer->host, version, remote_as, holdtime,
	       inet_ntoa (remote_id));
  if (CHECK_FLAG (peer->sflags, PEER_STATUS_ACCEPT_PEER))
    {
      int as = 0;
      realpeer = peer_lookup_with_open (&peer->su, remote_as, &remote_id, &as);
      if (! realpeer)
	{
	  if (as)
	    {
	      if (BGP_DEBUG (normal, NORMAL))
		zlog_debug ("%s bad OPEN, wrong router identifier %s",
			    peer->host, inet_ntoa (remote_id));
	      bgp_notify_send_with_data (peer, BGP_NOTIFY_OPEN_ERR, 
					 BGP_NOTIFY_OPEN_BAD_BGP_IDENT,
					 notify_data_remote_id, 4);
	    }
	  else
	    {
	      if (BGP_DEBUG (normal, NORMAL))
		zlog_debug ("%s bad OPEN, remote AS is %d, expected %d",
			    peer->host, remote_as, peer->as);
	      bgp_notify_send_with_data (peer, BGP_NOTIFY_OPEN_ERR,
					 BGP_NOTIFY_OPEN_BAD_PEER_AS,
					 notify_data_remote_as, 2);
	    }
	  return -1;
	}
    }
  ret = bgp_collision_detect (peer, remote_id);
  if (ret < 0)
    return ret;
  if (CHECK_FLAG (peer->sflags, PEER_STATUS_ACCEPT_PEER))
    {
	if (realpeer->status == Established
	    && CHECK_FLAG (realpeer->sflags, PEER_STATUS_NSF_MODE))
	{
	  realpeer->last_reset = PEER_DOWN_NSF_CLOSE_SESSION;
	  SET_FLAG (realpeer->sflags, PEER_STATUS_NSF_WAIT);
	}
	else if (ret == 0 && realpeer->status != Active
	         && realpeer->status != OpenSent
		 && realpeer->status != OpenConfirm)
 	{
 	  if (BGP_DEBUG (events, EVENTS))
	    zlog_debug ("%s peer status is %s close connection",
			realpeer->host, LOOKUP (bgp_status_msg,
			realpeer->status));
	  bgp_notify_send (peer, BGP_NOTIFY_CEASE,
			   BGP_NOTIFY_CEASE_CONNECT_REJECT);
 	  return -1;
 	}
      if (BGP_DEBUG (events, EVENTS))
	zlog_debug ("%s [Event] Transfer temporary BGP peer to existing one",
		   peer->host);
      bgp_stop (realpeer);
      realpeer->fd = peer->fd;
      peer->fd = -1;
      stream_free (realpeer->ibuf);
      realpeer->ibuf = peer->ibuf;
      realpeer->packet_size = peer->packet_size;
      peer->ibuf = NULL;
      realpeer->status = peer->status;
      bgp_stop (peer);
      peer = realpeer;
      bgp_open_send (peer);
      if (peer->fd < 0)
	{
	  zlog_err ("bgp_open_receive peer's fd is negative value %d",
		    peer->fd);
	  return -1;
	}
      BGP_READ_ON (peer->t_read, bgp_read, peer->fd);
    }
  if (remote_id.s_addr == 0
      || ntohl (remote_id.s_addr) >= 0xe0000000
      || ntohl (peer->local_id.s_addr) == ntohl (remote_id.s_addr))
    {
      if (BGP_DEBUG (normal, NORMAL))
	zlog_debug ("%s bad OPEN, wrong router identifier %s",
		   peer->host, inet_ntoa (remote_id));
      bgp_notify_send_with_data (peer, 
				 BGP_NOTIFY_OPEN_ERR, 
				 BGP_NOTIFY_OPEN_BAD_BGP_IDENT,
				 notify_data_remote_id, 4);
      return -1;
    }
  peer->remote_id = remote_id;
  if (version != BGP_VERSION_4)
    {
      u_int8_t maxver = BGP_VERSION_4;
      if (BGP_DEBUG (normal, NORMAL))
	zlog_debug ("%s bad protocol version, remote requested %d, local request %d",
		   peer->host, version, BGP_VERSION_4);
      bgp_notify_send_with_data (peer, 
				 BGP_NOTIFY_OPEN_ERR, 
				 BGP_NOTIFY_OPEN_UNSUP_VERSION,
				 &maxver, 1);
      return -1;
    }
  if (remote_as != peer->as)
    {
      if (BGP_DEBUG (normal, NORMAL))
	zlog_debug ("%s bad OPEN, remote AS is %d, expected %d",
		   peer->host, remote_as, peer->as);
      bgp_notify_send_with_data (peer, 
				 BGP_NOTIFY_OPEN_ERR, 
				 BGP_NOTIFY_OPEN_BAD_PEER_AS,
				 notify_data_remote_as, 2);
      return -1;
    }
  if (holdtime < 3 && holdtime != 0)
    {
      bgp_notify_send (peer,
		       BGP_NOTIFY_OPEN_ERR, 
		       BGP_NOTIFY_OPEN_UNACEP_HOLDTIME);
      return -1;
    }
  if (CHECK_FLAG (peer->config, PEER_CONFIG_TIMER))
    send_holdtime = peer->holdtime;
  else
    send_holdtime = peer->bgp->default_holdtime;
  if (holdtime < send_holdtime)
    peer->v_holdtime = holdtime;
  else
    peer->v_holdtime = send_holdtime;
  peer->v_keepalive = peer->v_holdtime / 3;
  capability = 0;
  optlen = stream_getc (peer->ibuf);
  if (optlen != 0) 
    {
      ret = bgp_open_option_parse (peer, optlen, &capability);
      if (ret < 0)
	return ret;
      stream_forward_getp (peer->ibuf, optlen);
    }
  else
    {
      if (BGP_DEBUG (normal, NORMAL))
	zlog_debug ("%s rcvd OPEN w/ OPTION parameter len: 0",
		   peer->host);
    }
  if (! capability || CHECK_FLAG (peer->flags, PEER_FLAG_OVERRIDE_CAPABILITY))
    {
      peer->afc_nego[AFI_IP][SAFI_UNICAST] = peer->afc[AFI_IP][SAFI_UNICAST];
      peer->afc_nego[AFI_IP][SAFI_MULTICAST] = peer->afc[AFI_IP][SAFI_MULTICAST];
      peer->afc_nego[AFI_IP6][SAFI_UNICAST] = peer->afc[AFI_IP6][SAFI_UNICAST];
      peer->afc_nego[AFI_IP6][SAFI_MULTICAST] = peer->afc[AFI_IP6][SAFI_MULTICAST];
    }
  bgp_getsockname (peer);
  BGP_EVENT_ADD (peer, Receive_OPEN_message);
  peer->packet_size = 0;
  if (peer->ibuf)
    stream_reset (peer->ibuf);
  return 0;
}