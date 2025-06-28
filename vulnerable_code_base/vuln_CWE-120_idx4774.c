static int __pyx_pf_17clickhouse_driver_14bufferedreader_14BufferedReader_19current_buffer_size_2__set__(struct __pyx_obj_17clickhouse_driver_14bufferedreader_BufferedReader *__pyx_v_self, PyObject *__pyx_v_value) {
  int __pyx_r;
  __Pyx_RefNannyDeclarations
  Py_ssize_t __pyx_t_1;
  __Pyx_RefNannySetupContext("__set__", 0);
  __pyx_t_1 = __Pyx_PyIndex_AsSsize_t(__pyx_v_value); if (unlikely((__pyx_t_1 == (Py_ssize_t)-1) && PyErr_Occurred())) __PYX_ERR(0, 11, __pyx_L1_error)
  __pyx_v_self->current_buffer_size = __pyx_t_1;
  __pyx_r = 0;
  goto __pyx_L0;
  __pyx_L1_error:;
  __Pyx_AddTraceback("clickhouse_driver.bufferedreader.BufferedReader.current_buffer_size.__set__", __pyx_clineno, __pyx_lineno, __pyx_filename);
  __pyx_r = -1;
  __pyx_L0:;
  __Pyx_RefNannyFinishContext();
  return __pyx_r;
}