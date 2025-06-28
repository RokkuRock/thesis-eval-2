static int __pyx_pf_17clickhouse_driver_14bufferedwriter_14BufferedWriter___init__(struct __pyx_obj_17clickhouse_driver_14bufferedwriter_BufferedWriter *__pyx_v_self, Py_ssize_t __pyx_v_bufsize) {
  int __pyx_r;
  __Pyx_RefNannyDeclarations
  int __pyx_t_1;
  PyObject *__pyx_t_2 = NULL;
  PyObject *__pyx_t_3 = NULL;
  PyObject *__pyx_t_4 = NULL;
  __Pyx_RefNannySetupContext("__init__", 0);
  __pyx_v_self->buffer = ((char *)PyMem_Malloc(__pyx_v_bufsize));
  __pyx_t_1 = ((!(__pyx_v_self->buffer != 0)) != 0);
  if (unlikely(__pyx_t_1)) {
    PyErr_NoMemory(); __PYX_ERR(0, 15, __pyx_L1_error)
  }
  __pyx_v_self->position = 0;
  __pyx_v_self->buffer_size = __pyx_v_bufsize;
  __pyx_t_3 = PyTuple_New(2); if (unlikely(!__pyx_t_3)) __PYX_ERR(0, 20, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_3);
  __Pyx_INCREF(((PyObject *)__pyx_ptype_17clickhouse_driver_14bufferedwriter_BufferedWriter));
  __Pyx_GIVEREF(((PyObject *)__pyx_ptype_17clickhouse_driver_14bufferedwriter_BufferedWriter));
  PyTuple_SET_ITEM(__pyx_t_3, 0, ((PyObject *)__pyx_ptype_17clickhouse_driver_14bufferedwriter_BufferedWriter));
  __Pyx_INCREF(((PyObject *)__pyx_v_self));
  __Pyx_GIVEREF(((PyObject *)__pyx_v_self));
  PyTuple_SET_ITEM(__pyx_t_3, 1, ((PyObject *)__pyx_v_self));
  __pyx_t_4 = __Pyx_PyObject_Call(__pyx_builtin_super, __pyx_t_3, NULL); if (unlikely(!__pyx_t_4)) __PYX_ERR(0, 20, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_4);
  __Pyx_DECREF(__pyx_t_3); __pyx_t_3 = 0;
  __pyx_t_3 = __Pyx_PyObject_GetAttrStr(__pyx_t_4, __pyx_n_s_init); if (unlikely(!__pyx_t_3)) __PYX_ERR(0, 20, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_3);
  __Pyx_DECREF(__pyx_t_4); __pyx_t_4 = 0;
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
  if (unlikely(!__pyx_t_2)) __PYX_ERR(0, 20, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_2);
  __Pyx_DECREF(__pyx_t_3); __pyx_t_3 = 0;
  __Pyx_DECREF(__pyx_t_2); __pyx_t_2 = 0;
  __pyx_r = 0;
  goto __pyx_L0;
  __pyx_L1_error:;
  __Pyx_XDECREF(__pyx_t_2);
  __Pyx_XDECREF(__pyx_t_3);
  __Pyx_XDECREF(__pyx_t_4);
  __Pyx_AddTraceback("clickhouse_driver.bufferedwriter.BufferedWriter.__init__", __pyx_clineno, __pyx_lineno, __pyx_filename);
  __pyx_r = -1;
  __pyx_L0:;
  __Pyx_RefNannyFinishContext();
  return __pyx_r;
}