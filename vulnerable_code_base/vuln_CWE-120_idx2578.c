static PyObject *__pyx_pw_17clickhouse_driver_6varint_1write_varint(PyObject *__pyx_self, PyObject *__pyx_args, PyObject *__pyx_kwds) {
  Py_ssize_t __pyx_v_number;
  PyObject *__pyx_v_buf = 0;
  PyObject *__pyx_r = 0;
  __Pyx_RefNannyDeclarations
  __Pyx_RefNannySetupContext("write_varint (wrapper)", 0);
  {
    static PyObject **__pyx_pyargnames[] = {&__pyx_n_s_number,&__pyx_n_s_buf,0};
    PyObject* values[2] = {0,0};
    if (unlikely(__pyx_kwds)) {
      Py_ssize_t kw_args;
      const Py_ssize_t pos_args = PyTuple_GET_SIZE(__pyx_args);
      switch (pos_args) {
        case  2: values[1] = PyTuple_GET_ITEM(__pyx_args, 1);
        CYTHON_FALLTHROUGH;
        case  1: values[0] = PyTuple_GET_ITEM(__pyx_args, 0);
        CYTHON_FALLTHROUGH;
        case  0: break;
        default: goto __pyx_L5_argtuple_error;
      }
      kw_args = PyDict_Size(__pyx_kwds);
      switch (pos_args) {
        case  0:
        if (likely((values[0] = __Pyx_PyDict_GetItemStr(__pyx_kwds, __pyx_n_s_number)) != 0)) kw_args--;
        else goto __pyx_L5_argtuple_error;
        CYTHON_FALLTHROUGH;
        case  1:
        if (likely((values[1] = __Pyx_PyDict_GetItemStr(__pyx_kwds, __pyx_n_s_buf)) != 0)) kw_args--;
        else {
          __Pyx_RaiseArgtupleInvalid("write_varint", 1, 2, 2, 1); __PYX_ERR(0, 4, __pyx_L3_error)
        }
      }
      if (unlikely(kw_args > 0)) {
        if (unlikely(__Pyx_ParseOptionalKeywords(__pyx_kwds, __pyx_pyargnames, 0, values, pos_args, "write_varint") < 0)) __PYX_ERR(0, 4, __pyx_L3_error)
      }
    } else if (PyTuple_GET_SIZE(__pyx_args) != 2) {
      goto __pyx_L5_argtuple_error;
    } else {
      values[0] = PyTuple_GET_ITEM(__pyx_args, 0);
      values[1] = PyTuple_GET_ITEM(__pyx_args, 1);
    }
    __pyx_v_number = __Pyx_PyIndex_AsSsize_t(values[0]); if (unlikely((__pyx_v_number == (Py_ssize_t)-1) && PyErr_Occurred())) __PYX_ERR(0, 4, __pyx_L3_error)
    __pyx_v_buf = values[1];
  }
  goto __pyx_L4_argument_unpacking_done;
  __pyx_L5_argtuple_error:;
  __Pyx_RaiseArgtupleInvalid("write_varint", 1, 2, 2, PyTuple_GET_SIZE(__pyx_args)); __PYX_ERR(0, 4, __pyx_L3_error)
  __pyx_L3_error:;
  __Pyx_AddTraceback("clickhouse_driver.varint.write_varint", __pyx_clineno, __pyx_lineno, __pyx_filename);
  __Pyx_RefNannyFinishContext();
  return NULL;
  __pyx_L4_argument_unpacking_done:;
  __pyx_r = __pyx_pf_17clickhouse_driver_6varint_write_varint(__pyx_self, __pyx_v_number, __pyx_v_buf);
  __Pyx_RefNannyFinishContext();
  return __pyx_r;
}