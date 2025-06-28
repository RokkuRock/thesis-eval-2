static PyObject *__pyx_pw_17clickhouse_driver_14bufferedreader_14BufferedReader_5read(PyObject *__pyx_v_self, PyObject *__pyx_arg_unread) {
  Py_ssize_t __pyx_v_unread;
  PyObject *__pyx_r = 0;
  __Pyx_RefNannyDeclarations
  __Pyx_RefNannySetupContext("read (wrapper)", 0);
  assert(__pyx_arg_unread); {
    __pyx_v_unread = __Pyx_PyIndex_AsSsize_t(__pyx_arg_unread); if (unlikely((__pyx_v_unread == (Py_ssize_t)-1) && PyErr_Occurred())) __PYX_ERR(0, 25, __pyx_L3_error)
  }
  goto __pyx_L4_argument_unpacking_done;
  __pyx_L3_error:;
  __Pyx_AddTraceback("clickhouse_driver.bufferedreader.BufferedReader.read", __pyx_clineno, __pyx_lineno, __pyx_filename);
  __Pyx_RefNannyFinishContext();
  return NULL;
  __pyx_L4_argument_unpacking_done:;
  __pyx_r = __pyx_pf_17clickhouse_driver_14bufferedreader_14BufferedReader_4read(((struct __pyx_obj_17clickhouse_driver_14bufferedreader_BufferedReader *)__pyx_v_self), ((Py_ssize_t)__pyx_v_unread));
  __Pyx_RefNannyFinishContext();
  return __pyx_r;
}