static PyObject *__pyx_pf_17clickhouse_driver_6varint_write_varint(CYTHON_UNUSED PyObject *__pyx_self, Py_ssize_t __pyx_v_number, PyObject *__pyx_v_buf) {
  Py_ssize_t __pyx_v_i;
  unsigned char __pyx_v_towrite;
  unsigned char __pyx_v_num_buf[32];
  PyObject *__pyx_r = NULL;
  __Pyx_RefNannyDeclarations
  int __pyx_t_1;
  PyObject *__pyx_t_2 = NULL;
  PyObject *__pyx_t_3 = NULL;
  PyObject *__pyx_t_4 = NULL;
  PyObject *__pyx_t_5 = NULL;
  __Pyx_RefNannySetupContext("write_varint", 0);
  __pyx_v_i = 0;
  while (1) {
    __pyx_v_towrite = (__pyx_v_number & 0x7f);
    __pyx_v_number = (__pyx_v_number >> 7);
    __pyx_t_1 = (__pyx_v_number != 0);
    if (__pyx_t_1) {
      (__pyx_v_num_buf[__pyx_v_i]) = (__pyx_v_towrite | 0x80);
      __pyx_v_i = (__pyx_v_i + 1);
      goto __pyx_L5;
    }
      {
      (__pyx_v_num_buf[__pyx_v_i]) = __pyx_v_towrite;
      __pyx_v_i = (__pyx_v_i + 1);
      goto __pyx_L4_break;
    }
    __pyx_L5:;
  }
  __pyx_L4_break:;
  __pyx_t_3 = __Pyx_PyObject_GetAttrStr(__pyx_v_buf, __pyx_n_s_write); if (unlikely(!__pyx_t_3)) __PYX_ERR(0, 26, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_3);
  __pyx_t_4 = PyBytes_FromStringAndSize(((char *)__pyx_v_num_buf), __pyx_v_i); if (unlikely(!__pyx_t_4)) __PYX_ERR(0, 26, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_4);
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
  __pyx_t_2 = (__pyx_t_5) ? __Pyx_PyObject_Call2Args(__pyx_t_3, __pyx_t_5, __pyx_t_4) : __Pyx_PyObject_CallOneArg(__pyx_t_3, __pyx_t_4);
  __Pyx_XDECREF(__pyx_t_5); __pyx_t_5 = 0;
  __Pyx_DECREF(__pyx_t_4); __pyx_t_4 = 0;
  if (unlikely(!__pyx_t_2)) __PYX_ERR(0, 26, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_2);
  __Pyx_DECREF(__pyx_t_3); __pyx_t_3 = 0;
  __Pyx_DECREF(__pyx_t_2); __pyx_t_2 = 0;
  __pyx_r = Py_None; __Pyx_INCREF(Py_None);
  goto __pyx_L0;
  __pyx_L1_error:;
  __Pyx_XDECREF(__pyx_t_2);
  __Pyx_XDECREF(__pyx_t_3);
  __Pyx_XDECREF(__pyx_t_4);
  __Pyx_XDECREF(__pyx_t_5);
  __Pyx_AddTraceback("clickhouse_driver.varint.write_varint", __pyx_clineno, __pyx_lineno, __pyx_filename);
  __pyx_r = NULL;
  __pyx_L0:;
  __Pyx_XGIVEREF(__pyx_r);
  __Pyx_RefNannyFinishContext();
  return __pyx_r;
}