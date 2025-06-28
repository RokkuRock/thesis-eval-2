static memcached_return_t _read_one_response(memcached_instance_st *instance, char *buffer,
                                             const size_t buffer_length,
                                             memcached_result_st *result) {
  memcached_server_response_decrement(instance);
  if (result == NULL) {
    Memcached *root = (Memcached *) instance->root;
    result = &root->result;
  }
  memcached_return_t rc;
  if (memcached_is_binary(instance->root)) {
    do {
      rc = binary_read_one_response(instance, buffer, buffer_length, result);
    } while (rc == MEMCACHED_FETCH_NOTFINISHED);
  } else {
    rc = textual_read_one_response(instance, buffer, buffer_length, result);
  }
  if (memcached_fatal(rc) && rc != MEMCACHED_TIMEOUT) {
    memcached_io_reset(instance);
  }
  return rc;
}