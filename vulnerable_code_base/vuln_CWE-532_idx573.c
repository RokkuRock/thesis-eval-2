rend_service_intro_established(origin_circuit_t *circuit,
                               const uint8_t *request,
                               size_t request_len)
{
  rend_service_t *service;
  rend_intro_point_t *intro;
  char serviceid[REND_SERVICE_ID_LEN_BASE32+1];
  (void) request;
  (void) request_len;
  tor_assert(circuit->rend_data);
  const char *rend_pk_digest =
    (char *) rend_data_get_pk_digest(circuit->rend_data, NULL);
  if (circuit->base_.purpose != CIRCUIT_PURPOSE_S_ESTABLISH_INTRO) {
    log_warn(LD_PROTOCOL,
             "received INTRO_ESTABLISHED cell on non-intro circuit.");
    goto err;
  }
  service = rend_service_get_by_pk_digest(rend_pk_digest);
  if (!service) {
    log_warn(LD_REND, "Unknown service on introduction circuit %u.",
             (unsigned)circuit->base_.n_circ_id);
    goto err;
  }
  intro = find_intro_point(circuit);
  if (intro == NULL) {
    log_warn(LD_REND,
             "Introduction circuit established without a rend_intro_point_t "
             "object for service %s on circuit %u",
             safe_str_client(serviceid), (unsigned)circuit->base_.n_circ_id);
    goto err;
  }
  intro->circuit_established = 1;
  service->desc_is_dirty = time(NULL);
  circuit_change_purpose(TO_CIRCUIT(circuit), CIRCUIT_PURPOSE_S_INTRO);
  base32_encode(serviceid, REND_SERVICE_ID_LEN_BASE32 + 1,
                rend_pk_digest, REND_SERVICE_ID_LEN);
  log_info(LD_REND,
           "Received INTRO_ESTABLISHED cell on circuit %u for service %s",
           (unsigned)circuit->base_.n_circ_id, serviceid);
  pathbias_mark_use_success(circuit);
  return 0;
 err:
  circuit_mark_for_close(TO_CIRCUIT(circuit), END_CIRC_REASON_TORPROTOCOL);
  return -1;
}