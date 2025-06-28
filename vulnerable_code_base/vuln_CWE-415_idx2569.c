static int csnmp_read_table(host_definition_t *host, data_definition_t *data) {
  struct snmp_pdu *req;
  struct snmp_pdu *res = NULL;
  struct variable_list *vb;
  const data_set_t *ds;
  size_t oid_list_len = data->values_len + 1;
  oid_t oid_list[oid_list_len];
  _Bool oid_list_todo[oid_list_len];
  int status;
  size_t i;
  csnmp_list_instances_t *instance_list_head;
  csnmp_list_instances_t *instance_list_tail;
  csnmp_table_values_t **value_list_head;
  csnmp_table_values_t **value_list_tail;
  DEBUG("snmp plugin: csnmp_read_table (host = %s, data = %s)", host->name,
        data->name);
  if (host->sess_handle == NULL) {
    DEBUG("snmp plugin: csnmp_read_table: host->sess_handle == NULL");
    return (-1);
  }
  ds = plugin_get_ds(data->type);
  if (!ds) {
    ERROR("snmp plugin: DataSet `%s' not defined.", data->type);
    return (-1);
  }
  if (ds->ds_num != data->values_len) {
    ERROR("snmp plugin: DataSet `%s' requires %zu values, but config talks "
          "about %zu",
          data->type, ds->ds_num, data->values_len);
    return (-1);
  }
  assert(data->values_len > 0);
  memcpy(oid_list, data->values, data->values_len * sizeof(oid_t));
  if (data->instance.oid.oid_len > 0)
    memcpy(oid_list + data->values_len, &data->instance.oid, sizeof(oid_t));
  else  
    oid_list_len--;
  for (i = 0; i < oid_list_len; i++)
    oid_list_todo[i] = 1;
  value_list_head = calloc(data->values_len, sizeof(*value_list_head));
  value_list_tail = calloc(data->values_len, sizeof(*value_list_tail));
  if ((value_list_head == NULL) || (value_list_tail == NULL)) {
    ERROR("snmp plugin: csnmp_read_table: calloc failed.");
    sfree(value_list_head);
    sfree(value_list_tail);
    return (-1);
  }
  instance_list_head = NULL;
  instance_list_tail = NULL;
  status = 0;
  while (status == 0) {
    int oid_list_todo_num;
    req = snmp_pdu_create(SNMP_MSG_GETNEXT);
    if (req == NULL) {
      ERROR("snmp plugin: snmp_pdu_create failed.");
      status = -1;
      break;
    }
    oid_list_todo_num = 0;
    for (i = 0; i < oid_list_len; i++) {
      if (!oid_list_todo[i])
        continue;
      oid_list_todo_num++;
      snmp_add_null_var(req, oid_list[i].oid, oid_list[i].oid_len);
    }
    if (oid_list_todo_num == 0) {
      DEBUG("snmp plugin: all variables have left their subtree");
      status = 0;
      break;
    }
    res = NULL;
    status = snmp_sess_synch_response(host->sess_handle, req, &res);
    if ((status != STAT_SUCCESS) || (res == NULL)) {
      char *errstr = NULL;
      snmp_sess_error(host->sess_handle, NULL, NULL, &errstr);
      c_complain(LOG_ERR, &host->complaint,
                 "snmp plugin: host %s: snmp_sess_synch_response failed: %s",
                 host->name, (errstr == NULL) ? "Unknown problem" : errstr);
      if (res != NULL)
        snmp_free_pdu(res);
      res = NULL;
      req = NULL;
      sfree(errstr);
      csnmp_host_close_session(host);
      status = -1;
      break;
    }
    status = 0;
    assert(res != NULL);
    c_release(LOG_INFO, &host->complaint,
              "snmp plugin: host %s: snmp_sess_synch_response successful.",
              host->name);
    vb = res->variables;
    if (vb == NULL) {
      status = -1;
      break;
    }
    for (vb = res->variables, i = 0; (vb != NULL);
         vb = vb->next_variable, i++) {
      while ((i < oid_list_len) && !oid_list_todo[i])
        i++;
      if ((data->instance.oid.oid_len > 0) && (i == data->values_len)) {
        if ((vb->type == SNMP_ENDOFMIBVIEW) ||
            (snmp_oid_ncompare(
                 data->instance.oid.oid, data->instance.oid.oid_len, vb->name,
                 vb->name_length, data->instance.oid.oid_len) != 0)) {
          DEBUG("snmp plugin: host = %s; data = %s; Instance left its subtree.",
                host->name, data->name);
          oid_list_todo[i] = 0;
          continue;
        }
        if (csnmp_instance_list_add(&instance_list_head, &instance_list_tail,
                                    res, host, data) != 0) {
          ERROR("snmp plugin: host %s: csnmp_instance_list_add failed.",
                host->name);
          status = -1;
          break;
        }
      } else  
      {
        csnmp_table_values_t *vt;
        oid_t vb_name;
        oid_t suffix;
        int ret;
        csnmp_oid_init(&vb_name, vb->name, vb->name_length);
        ret = csnmp_oid_suffix(&suffix, &vb_name, data->values + i);
        if (ret != 0) {
          DEBUG("snmp plugin: host = %s; data = %s; i = %zu; "
                "Value probably left its subtree.",
                host->name, data->name, i);
          oid_list_todo[i] = 0;
          continue;
        }
        if ((value_list_tail[i] != NULL) &&
            (csnmp_oid_compare(&suffix, &value_list_tail[i]->suffix) <= 0)) {
          DEBUG("snmp plugin: host = %s; data = %s; i = %zu; "
                "Suffix is not increasing.",
                host->name, data->name, i);
          oid_list_todo[i] = 0;
          continue;
        }
        vt = calloc(1, sizeof(*vt));
        if (vt == NULL) {
          ERROR("snmp plugin: calloc failed.");
          status = -1;
          break;
        }
        vt->value =
            csnmp_value_list_to_value(vb, ds->ds[i].type, data->scale,
                                      data->shift, host->name, data->name);
        memcpy(&vt->suffix, &suffix, sizeof(vt->suffix));
        vt->next = NULL;
        if (value_list_tail[i] == NULL)
          value_list_head[i] = vt;
        else
          value_list_tail[i]->next = vt;
        value_list_tail[i] = vt;
      }
      memcpy(oid_list[i].oid, vb->name, sizeof(oid) * vb->name_length);
      oid_list[i].oid_len = vb->name_length;
    }  
    if (res != NULL)
      snmp_free_pdu(res);
    res = NULL;
  }  
  if (res != NULL)
    snmp_free_pdu(res);
  res = NULL;
  if (req != NULL)
    snmp_free_pdu(req);
  req = NULL;
  if (status == 0)
    csnmp_dispatch_table(host, data, instance_list_head, value_list_head);
  while (instance_list_head != NULL) {
    csnmp_list_instances_t *next = instance_list_head->next;
    sfree(instance_list_head);
    instance_list_head = next;
  }
  for (i = 0; i < data->values_len; i++) {
    while (value_list_head[i] != NULL) {
      csnmp_table_values_t *next = value_list_head[i]->next;
      sfree(value_list_head[i]);
      value_list_head[i] = next;
    }
  }
  sfree(value_list_head);
  sfree(value_list_tail);
  return (0);
}  