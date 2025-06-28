static PyObject *__pyx_pf_17clickhouse_driver_6varint_2read_varint(CYTHON_UNUSED PyObject *__pyx_self, PyObject *__pyx_v_f) {
  Py_ssize_t __pyx_v_shift;
  Py_ssize_t __pyx_v_result;
  unsigned char __pyx_v_i;
  PyObject *__pyx_v_read_one = NULL;
  PyObject *__pyx_r = NULL;
  __Pyx_RefNannyDeclarations
  PyObject *__pyx_t_1 = NULL;
  PyObject *__pyx_t_2 = NULL;
  PyObject *__pyx_t_3 = NULL;
  unsigned char __pyx_t_4;
  int __pyx_t_5;
  __Pyx_RefNannySetupContext("read_varint", 0);
  __pyx_v_shift = 0;
  __pyx_v_result = 0;
  __pyx_t_1 = __Pyx_PyObject_GetAttrStr(__pyx_v_f, __pyx_n_s_read_one); if (unlikely(!__pyx_t_1)) __PYX_ERR(0, 37, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_1);
  __pyx_v_read_one = __pyx_t_1;
  __pyx_t_1 = 0;
  while (1) {
    __Pyx_INCREF(__pyx_v_read_one);
    __pyx_t_2 = __pyx_v_read_one; __pyx_t_3 = NULL;
    if (CYTHON_UNPACK_METHODS && likely(PyMethod_Check(__pyx_t_2))) {
      __pyx_t_3 = PyMethod_GET_SELF(__pyx_t_2);
      if (likely(__pyx_t_3)) {
        PyObject* function = PyMethod_GET_FUNCTION(__pyx_t_2);
        __Pyx_INCREF(__pyx_t_3);
        __Pyx_INCREF(function);
        __Pyx_DECREF_SET(__pyx_t_2, function);
      }
    }
    __pyx_t_1 = (__pyx_t_3) ? __Pyx_PyObject_CallOneArg(__pyx_t_2, __pyx_t_3) : __Pyx_PyObject_CallNoArg(__pyx_t_2);
    __Pyx_XDECREF(__pyx_t_3); __pyx_t_3 = 0;
    if (unlikely(!__pyx_t_1)) __PYX_ERR(0, 40, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_1);
    __Pyx_DECREF(__pyx_t_2); __pyx_t_2 = 0;
    __pyx_t_4 = __Pyx_PyInt_As_unsigned_char(__pyx_t_1); if (unlikely((__pyx_t_4 == (unsigned char)-1) && PyErr_Occurred())) __PYX_ERR(0, 40, __pyx_L1_error)
    __Pyx_DECREF(__pyx_t_1); __pyx_t_1 = 0;
    __pyx_v_i = __pyx_t_4;
    __pyx_v_result = (__pyx_v_result | ((__pyx_v_i & 0x7f) << __pyx_v_shift));
    __pyx_v_shift = (__pyx_v_shift + 7);
    __pyx_t_5 = ((__pyx_v_i < 0x80) != 0);
    if (__pyx_t_5) {
      goto __pyx_L4_break;
    }
  }
  __pyx_L4_break:;
  __Pyx_XDECREF(__pyx_r);
  __pyx_t_1 = PyInt_FromSsize_t(__pyx_v_result); if (unlikely(!__pyx_t_1)) __PYX_ERR(0, 46, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_1);
  __pyx_r = __pyx_t_1;
  __pyx_t_1 = 0;
  goto __pyx_L0;
  __pyx_L1_error:;
  __Pyx_XDECREF(__pyx_t_1);
  __Pyx_XDECREF(__pyx_t_2);
  __Pyx_XDECREF(__pyx_t_3);
  __Pyx_AddTraceback("clickhouse_driver.varint.read_varint", __pyx_clineno, __pyx_lineno, __pyx_filename);
  __pyx_r = NULL;
  __pyx_L0:;
  __Pyx_XDECREF(__pyx_v_read_one);
  __Pyx_XGIVEREF(__pyx_r);
  __Pyx_RefNannyFinishContext();
  return __pyx_r;
}