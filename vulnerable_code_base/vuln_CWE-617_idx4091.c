connection_exit_begin_conn(cell_t *cell, circuit_t *circ)
{
  edge_connection_t *n_stream;
  relay_header_t rh;
  char *address = NULL;
  uint16_t port = 0;
  or_circuit_t *or_circ = NULL;
  const or_options_t *options = get_options();
  begin_cell_t bcell;
  int rv;
  uint8_t end_reason=0;
  assert_circuit_ok(circ);
  if (!CIRCUIT_IS_ORIGIN(circ))
    or_circ = TO_OR_CIRCUIT(circ);
  relay_header_unpack(&rh, cell->payload);
  if (rh.length > RELAY_PAYLOAD_SIZE)
    return -END_CIRC_REASON_TORPROTOCOL;
  if (!server_mode(options) &&
      circ->purpose != CIRCUIT_PURPOSE_S_REND_JOINED) {
    log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
           "Relay begin cell at non-server. Closing.");
    relay_send_end_cell_from_edge(rh.stream_id, circ,
                                  END_STREAM_REASON_EXITPOLICY, NULL);
    return 0;
  }
  rv = begin_cell_parse(cell, &bcell, &end_reason);
  if (rv < -1) {
    return -END_CIRC_REASON_TORPROTOCOL;
  } else if (rv == -1) {
    tor_free(bcell.address);
    relay_send_end_cell_from_edge(rh.stream_id, circ, end_reason, NULL);
    return 0;
  }
  if (! bcell.is_begindir) {
    address = bcell.address;
    port = bcell.port;
    if (or_circ && or_circ->p_chan) {
      if (!options->AllowSingleHopExits &&
           (or_circ->is_first_hop ||
            (!connection_or_digest_is_known_relay(
                or_circ->p_chan->identity_digest) &&
          should_refuse_unknown_exits(options)))) {
        log_fn(LOG_PROTOCOL_WARN, LD_PROTOCOL,
               "Attempt by %s to open a stream %s. Closing.",
               safe_str(channel_get_canonical_remote_descr(or_circ->p_chan)),
               or_circ->is_first_hop ? "on first hop of circuit" :
                                       "from unknown relay");
        relay_send_end_cell_from_edge(rh.stream_id, circ,
                                      or_circ->is_first_hop ?
                                        END_STREAM_REASON_TORPROTOCOL :
                                        END_STREAM_REASON_MISC,
                                      NULL);
        tor_free(address);
        return 0;
      }
    }
  } else if (rh.command == RELAY_COMMAND_BEGIN_DIR) {
    if (!directory_permits_begindir_requests(options) ||
        circ->purpose != CIRCUIT_PURPOSE_OR) {
      relay_send_end_cell_from_edge(rh.stream_id, circ,
                                    END_STREAM_REASON_NOTDIRECTORY, NULL);
      return 0;
    }
    if (or_circ && or_circ->p_chan)
      address = tor_strdup(channel_get_actual_remote_address(or_circ->p_chan));
    else
      address = tor_strdup("127.0.0.1");
    port = 1;  
  } else {
    log_warn(LD_BUG, "Got an unexpected command %d", (int)rh.command);
    relay_send_end_cell_from_edge(rh.stream_id, circ,
                                  END_STREAM_REASON_INTERNAL, NULL);
    return 0;
  }
  if (! options->IPv6Exit) {
    bcell.flags &= ~BEGIN_FLAG_IPV6_PREFERRED;
    if (bcell.flags & BEGIN_FLAG_IPV4_NOT_OK) {
      tor_free(address);
      relay_send_end_cell_from_edge(rh.stream_id, circ,
                                    END_STREAM_REASON_EXITPOLICY, NULL);
      return 0;
    }
  }
  log_debug(LD_EXIT,"Creating new exit connection.");
  n_stream = edge_connection_new(CONN_TYPE_EXIT, AF_INET);
  n_stream->dirreq_id = circ->dirreq_id;
  n_stream->base_.purpose = EXIT_PURPOSE_CONNECT;
  n_stream->begincell_flags = bcell.flags;
  n_stream->stream_id = rh.stream_id;
  n_stream->base_.port = port;
  n_stream->package_window = STREAMWINDOW_START;
  n_stream->deliver_window = STREAMWINDOW_START;
  if (circ->purpose == CIRCUIT_PURPOSE_S_REND_JOINED) {
    origin_circuit_t *origin_circ = TO_ORIGIN_CIRCUIT(circ);
    log_info(LD_REND,"begin is for rendezvous. configuring stream.");
    n_stream->base_.address = tor_strdup("(rendezvous)");
    n_stream->base_.state = EXIT_CONN_STATE_CONNECTING;
    n_stream->rend_data = rend_data_dup(origin_circ->rend_data);
    tor_assert(connection_edge_is_rendezvous_stream(n_stream));
    assert_circuit_ok(circ);
    const int r = rend_service_set_connection_addr_port(n_stream, origin_circ);
    if (r < 0) {
      log_info(LD_REND,"Didn't find rendezvous service (port %d)",
               n_stream->base_.port);
      relay_send_end_cell_from_edge(rh.stream_id, circ,
                                    END_STREAM_REASON_DONE,
                                    origin_circ->cpath->prev);
      connection_free(TO_CONN(n_stream));
      tor_free(address);
      if (r < -1)
        return END_CIRC_AT_ORIGIN;
      else
        return 0;
    }
    assert_circuit_ok(circ);
    log_debug(LD_REND,"Finished assigning addr/port");
    n_stream->cpath_layer = origin_circ->cpath->prev;  
    n_stream->next_stream = origin_circ->p_streams;
    n_stream->on_circuit = circ;
    origin_circ->p_streams = n_stream;
    assert_circuit_ok(circ);
    origin_circ->rend_data->nr_streams++;
    connection_exit_connect(n_stream);
    pathbias_mark_use_success(origin_circ);
    tor_free(address);
    return 0;
  }
  tor_strlower(address);
  n_stream->base_.address = address;
  n_stream->base_.state = EXIT_CONN_STATE_RESOLVEFAILED;
  if (we_are_hibernating()) {
    relay_send_end_cell_from_edge(rh.stream_id, circ,
                                  END_STREAM_REASON_HIBERNATING, NULL);
    connection_free(TO_CONN(n_stream));
    return 0;
  }
  n_stream->on_circuit = circ;
  if (rh.command == RELAY_COMMAND_BEGIN_DIR) {
    tor_addr_t tmp_addr;
    tor_assert(or_circ);
    if (or_circ->p_chan &&
        channel_get_addr_if_possible(or_circ->p_chan, &tmp_addr)) {
      tor_addr_copy(&n_stream->base_.addr, &tmp_addr);
    }
    return connection_exit_connect_dir(n_stream);
  }
  log_debug(LD_EXIT,"about to start the dns_resolve().");
  switch (dns_resolve(n_stream)) {
    case 1:  
      assert_circuit_ok(circ);
      log_debug(LD_EXIT,"about to call connection_exit_connect().");
      connection_exit_connect(n_stream);
      return 0;
    case -1:  
      relay_send_end_cell_from_edge(rh.stream_id, circ,
                                    END_STREAM_REASON_RESOLVEFAILED, NULL);
      break;
    case 0:  
      assert_circuit_ok(circ);
      break;
  }
  return 0;
}