static PyObject *__pyx_pf_17clickhouse_driver_14bufferedreader_20BufferedSocketReader_2read_into_buffer(struct __pyx_obj_17clickhouse_driver_14bufferedreader_BufferedSocketReader *__pyx_v_self) {
  PyObject *__pyx_r = NULL;
  __Pyx_RefNannyDeclarations
  PyObject *__pyx_t_1 = NULL;
  PyObject *__pyx_t_2 = NULL;
  PyObject *__pyx_t_3 = NULL;
  Py_ssize_t __pyx_t_4;
  int __pyx_t_5;
  __Pyx_RefNannySetupContext("read_into_buffer", 0);
  __pyx_t_2 = __Pyx_PyObject_GetAttrStr(__pyx_v_self->sock, __pyx_n_s_recv_into); if (unlikely(!__pyx_t_2)) __PYX_ERR(0, 188, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_2);
  __pyx_t_3 = NULL;
  if (CYTHON_UNPACK_METHODS && likely(PyMethod_Check(__pyx_t_2))) {
    __pyx_t_3 = PyMethod_GET_SELF(__pyx_t_2);
    if (likely(__pyx_t_3)) {
      PyObject* function = PyMethod_GET_FUNCTION(__pyx_t_2);
      __Pyx_INCREF(__pyx_t_3);
      __Pyx_INCREF(function);
      __Pyx_DECREF_SET(__pyx_t_2, function);
    }
  }
  __pyx_t_1 = (__pyx_t_3) ? __Pyx_PyObject_Call2Args(__pyx_t_2, __pyx_t_3, __pyx_v_self->__pyx_base.buffer) : __Pyx_PyObject_CallOneArg(__pyx_t_2, __pyx_v_self->__pyx_base.buffer);
  __Pyx_XDECREF(__pyx_t_3); __pyx_t_3 = 0;
  if (unlikely(!__pyx_t_1)) __PYX_ERR(0, 188, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_1);
  __Pyx_DECREF(__pyx_t_2); __pyx_t_2 = 0;
  __pyx_t_4 = __Pyx_PyIndex_AsSsize_t(__pyx_t_1); if (unlikely((__pyx_t_4 == (Py_ssize_t)-1) && PyErr_Occurred())) __PYX_ERR(0, 188, __pyx_L1_error)
  __Pyx_DECREF(__pyx_t_1); __pyx_t_1 = 0;
  __pyx_v_self->__pyx_base.current_buffer_size = __pyx_t_4;
  __pyx_t_5 = ((__pyx_v_self->__pyx_base.current_buffer_size == 0) != 0);
  if (unlikely(__pyx_t_5)) {
    __pyx_t_1 = __Pyx_PyObject_Call(__pyx_builtin_EOFError, __pyx_tuple_, NULL); if (unlikely(!__pyx_t_1)) __PYX_ERR(0, 191, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_1);
    __Pyx_Raise(__pyx_t_1, 0, 0, 0);
    __Pyx_DECREF(__pyx_t_1); __pyx_t_1 = 0;
    __PYX_ERR(0, 191, __pyx_L1_error)
  }
  __pyx_r = Py_None; __Pyx_INCREF(Py_None);
  goto __pyx_L0;
  __pyx_L1_error:;
  __Pyx_XDECREF(__pyx_t_1);
  __Pyx_XDECREF(__pyx_t_2);
  __Pyx_XDECREF(__pyx_t_3);
  __Pyx_AddTraceback("clickhouse_driver.bufferedreader.BufferedSocketReader.read_into_buffer", __pyx_clineno, __pyx_lineno, __pyx_filename);
  __pyx_r = NULL;
  __pyx_L0:;
  __Pyx_XGIVEREF(__pyx_r);
  __Pyx_RefNannyFinishContext();
  return __pyx_r;
}