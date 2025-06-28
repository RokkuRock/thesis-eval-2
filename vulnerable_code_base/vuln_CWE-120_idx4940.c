static PyObject *__pyx_pf_17clickhouse_driver_14bufferedreader_14BufferedReader_4read(struct __pyx_obj_17clickhouse_driver_14bufferedreader_BufferedReader *__pyx_v_self, Py_ssize_t __pyx_v_unread) {
  Py_ssize_t __pyx_v_next_position;
  Py_ssize_t __pyx_v_t;
  char *__pyx_v_buffer_ptr;
  Py_ssize_t __pyx_v_read_bytes;
  PyObject *__pyx_v_rv = NULL;
  PyObject *__pyx_r = NULL;
  __Pyx_RefNannyDeclarations
  int __pyx_t_1;
  Py_ssize_t __pyx_t_2;
  PyObject *__pyx_t_3 = NULL;
  PyObject *__pyx_t_4 = NULL;
  PyObject *__pyx_t_5 = NULL;
  Py_ssize_t __pyx_t_6;
  Py_ssize_t __pyx_t_7;
  __Pyx_RefNannySetupContext("read", 0);
  __pyx_v_next_position = (__pyx_v_unread + __pyx_v_self->position);
  __pyx_t_1 = ((__pyx_v_next_position < __pyx_v_self->current_buffer_size) != 0);
  if (__pyx_t_1) {
    __pyx_t_2 = __pyx_v_self->position;
    __pyx_v_t = __pyx_t_2;
    __pyx_v_self->position = __pyx_v_next_position;
    __Pyx_XDECREF(__pyx_r);
    if (unlikely(__pyx_v_self->buffer == Py_None)) {
      PyErr_SetString(PyExc_TypeError, "'NoneType' object is not subscriptable");
      __PYX_ERR(0, 32, __pyx_L1_error)
    }
    __pyx_t_3 = PySequence_GetSlice(__pyx_v_self->buffer, __pyx_v_t, __pyx_v_self->position); if (unlikely(!__pyx_t_3)) __PYX_ERR(0, 32, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_3);
    __pyx_t_4 = __Pyx_PyObject_CallOneArg(((PyObject *)(&PyBytes_Type)), __pyx_t_3); if (unlikely(!__pyx_t_4)) __PYX_ERR(0, 32, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_4);
    __Pyx_DECREF(__pyx_t_3); __pyx_t_3 = 0;
    __pyx_r = __pyx_t_4;
    __pyx_t_4 = 0;
    goto __pyx_L0;
  }
  __pyx_t_4 = __pyx_v_self->buffer;
  __Pyx_INCREF(__pyx_t_4);
  __pyx_v_buffer_ptr = PyByteArray_AsString(__pyx_t_4);
  __Pyx_DECREF(__pyx_t_4); __pyx_t_4 = 0;
  __pyx_t_4 = __Pyx_PyObject_CallNoArg(((PyObject *)(&PyBytes_Type))); if (unlikely(!__pyx_t_4)) __PYX_ERR(0, 36, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_4);
  __pyx_v_rv = ((PyObject*)__pyx_t_4);
  __pyx_t_4 = 0;
  while (1) {
    __pyx_t_1 = ((__pyx_v_unread > 0) != 0);
    if (!__pyx_t_1) break;
    __pyx_t_1 = ((__pyx_v_self->position == __pyx_v_self->current_buffer_size) != 0);
    if (__pyx_t_1) {
      __pyx_t_3 = __Pyx_PyObject_GetAttrStr(((PyObject *)__pyx_v_self), __pyx_n_s_read_into_buffer); if (unlikely(!__pyx_t_3)) __PYX_ERR(0, 40, __pyx_L1_error)
      __Pyx_GOTREF(__pyx_t_3);
      __pyx_t_5 = NULL;
      if (CYTHON_UNPACK_METHODS && likely(PyMethod_Check(__pyx_t_3))) {
        __pyx_t_5 = PyMethod_GET_SELF(__pyx_t_3);
        if (likely(__pyx_t_5)) {
          PyObject* function = PyMethod_GET_FUNCTION(__pyx_t_3);
          __Pyx_INCREF(__pyx_t_5);
          __Pyx_INCREF(function);
          __Pyx_DECREF_SET(__pyx_t_3, function);
        }
      }
      __pyx_t_4 = (__pyx_t_5) ? __Pyx_PyObject_CallOneArg(__pyx_t_3, __pyx_t_5) : __Pyx_PyObject_CallNoArg(__pyx_t_3);
      __Pyx_XDECREF(__pyx_t_5); __pyx_t_5 = 0;
      if (unlikely(!__pyx_t_4)) __PYX_ERR(0, 40, __pyx_L1_error)
      __Pyx_GOTREF(__pyx_t_4);
      __Pyx_DECREF(__pyx_t_3); __pyx_t_3 = 0;
      __Pyx_DECREF(__pyx_t_4); __pyx_t_4 = 0;
      __pyx_t_4 = __pyx_v_self->buffer;
      __Pyx_INCREF(__pyx_t_4);
      __pyx_v_buffer_ptr = PyByteArray_AsString(__pyx_t_4);
      __Pyx_DECREF(__pyx_t_4); __pyx_t_4 = 0;
      __pyx_v_self->position = 0;
    }
    __pyx_t_2 = (__pyx_v_self->current_buffer_size - __pyx_v_self->position);
    __pyx_t_6 = __pyx_v_unread;
    if (((__pyx_t_2 < __pyx_t_6) != 0)) {
      __pyx_t_7 = __pyx_t_2;
    } else {
      __pyx_t_7 = __pyx_t_6;
    }
    __pyx_v_read_bytes = __pyx_t_7;
    __pyx_t_4 = PyBytes_FromStringAndSize((&(__pyx_v_buffer_ptr[__pyx_v_self->position])), __pyx_v_read_bytes); if (unlikely(!__pyx_t_4)) __PYX_ERR(0, 45, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_4);
    __pyx_t_3 = PyNumber_InPlaceAdd(__pyx_v_rv, __pyx_t_4); if (unlikely(!__pyx_t_3)) __PYX_ERR(0, 45, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_3);
    __Pyx_DECREF(__pyx_t_4); __pyx_t_4 = 0;
    __Pyx_DECREF_SET(__pyx_v_rv, ((PyObject*)__pyx_t_3));
    __pyx_t_3 = 0;
    __pyx_v_self->position = (__pyx_v_self->position + __pyx_v_read_bytes);
    __pyx_v_unread = (__pyx_v_unread - __pyx_v_read_bytes);
  }
  __Pyx_XDECREF(__pyx_r);
  __Pyx_INCREF(__pyx_v_rv);
  __pyx_r = __pyx_v_rv;
  goto __pyx_L0;
  __pyx_L1_error:;
  __Pyx_XDECREF(__pyx_t_3);
  __Pyx_XDECREF(__pyx_t_4);
  __Pyx_XDECREF(__pyx_t_5);
  __Pyx_AddTraceback("clickhouse_driver.bufferedreader.BufferedReader.read", __pyx_clineno, __pyx_lineno, __pyx_filename);
  __pyx_r = NULL;
  __pyx_L0:;
  __Pyx_XDECREF(__pyx_v_rv);
  __Pyx_XGIVEREF(__pyx_r);
  __Pyx_RefNannyFinishContext();
  return __pyx_r;
}