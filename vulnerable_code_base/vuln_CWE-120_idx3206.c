static PyObject *__pyx_pf_17clickhouse_driver_14bufferedreader_14BufferedReader_6read_one(struct __pyx_obj_17clickhouse_driver_14bufferedreader_BufferedReader *__pyx_v_self) {
  unsigned char __pyx_v_rv;
  PyObject *__pyx_r = NULL;
  __Pyx_RefNannyDeclarations
  int __pyx_t_1;
  PyObject *__pyx_t_2 = NULL;
  PyObject *__pyx_t_3 = NULL;
  PyObject *__pyx_t_4 = NULL;
  int __pyx_t_5;
  __Pyx_RefNannySetupContext("read_one", 0);
  __pyx_t_1 = ((__pyx_v_self->position == __pyx_v_self->current_buffer_size) != 0);
  if (__pyx_t_1) {
    __pyx_t_3 = __Pyx_PyObject_GetAttrStr(((PyObject *)__pyx_v_self), __pyx_n_s_read_into_buffer); if (unlikely(!__pyx_t_3)) __PYX_ERR(0, 55, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_3);
    __pyx_t_4 = NULL;
    if (CYTHON_UNPACK_METHODS && likely(PyMethod_Check(__pyx_t_3))) {
      __pyx_t_4 = PyMethod_GET_SELF(__pyx_t_3);
      if (likely(__pyx_t_4)) {
        PyObject* function = PyMethod_GET_FUNCTION(__pyx_t_3);
        __Pyx_INCREF(__pyx_t_4);
        __Pyx_INCREF(function);
        __Pyx_DECREF_SET(__pyx_t_3, function);
      }
    }
    __pyx_t_2 = (__pyx_t_4) ? __Pyx_PyObject_CallOneArg(__pyx_t_3, __pyx_t_4) : __Pyx_PyObject_CallNoArg(__pyx_t_3);
    __Pyx_XDECREF(__pyx_t_4); __pyx_t_4 = 0;
    if (unlikely(!__pyx_t_2)) __PYX_ERR(0, 55, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_2);
    __Pyx_DECREF(__pyx_t_3); __pyx_t_3 = 0;
    __Pyx_DECREF(__pyx_t_2); __pyx_t_2 = 0;
    __pyx_v_self->position = 0;
  }
  __pyx_t_5 = __Pyx_GetItemInt_ByteArray(__pyx_v_self->buffer, __pyx_v_self->position, Py_ssize_t, 1, PyInt_FromSsize_t, 0, 1, 1); if (unlikely(__pyx_t_5 == -1)) __PYX_ERR(0, 58, __pyx_L1_error)
  __pyx_v_rv = __pyx_t_5;
  __pyx_v_self->position = (__pyx_v_self->position + 1);
  __Pyx_XDECREF(__pyx_r);
  __pyx_t_2 = __Pyx_PyInt_From_unsigned_char(__pyx_v_rv); if (unlikely(!__pyx_t_2)) __PYX_ERR(0, 60, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_2);
  __pyx_r = __pyx_t_2;
  __pyx_t_2 = 0;
  goto __pyx_L0;
  __pyx_L1_error:;
  __Pyx_XDECREF(__pyx_t_2);
  __Pyx_XDECREF(__pyx_t_3);
  __Pyx_XDECREF(__pyx_t_4);
  __Pyx_AddTraceback("clickhouse_driver.bufferedreader.BufferedReader.read_one", __pyx_clineno, __pyx_lineno, __pyx_filename);
  __pyx_r = NULL;
  __pyx_L0:;
  __Pyx_XGIVEREF(__pyx_r);
  __Pyx_RefNannyFinishContext();
  return __pyx_r;
}