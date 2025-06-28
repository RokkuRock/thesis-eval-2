static PyObject *__pyx_f_17clickhouse_driver_14bufferedreader___pyx_unpickle_BufferedSocketReader__set_state(struct __pyx_obj_17clickhouse_driver_14bufferedreader_BufferedSocketReader *__pyx_v___pyx_result, PyObject *__pyx_v___pyx_state) {
  PyObject *__pyx_r = NULL;
  __Pyx_RefNannyDeclarations
  PyObject *__pyx_t_1 = NULL;
  Py_ssize_t __pyx_t_2;
  int __pyx_t_3;
  int __pyx_t_4;
  int __pyx_t_5;
  PyObject *__pyx_t_6 = NULL;
  PyObject *__pyx_t_7 = NULL;
  PyObject *__pyx_t_8 = NULL;
  __Pyx_RefNannySetupContext("__pyx_unpickle_BufferedSocketReader__set_state", 0);
  if (unlikely(__pyx_v___pyx_state == Py_None)) {
    PyErr_SetString(PyExc_TypeError, "'NoneType' object is not subscriptable");
    __PYX_ERR(1, 12, __pyx_L1_error)
  }
  __pyx_t_1 = __Pyx_GetItemInt_Tuple(__pyx_v___pyx_state, 0, long, 1, __Pyx_PyInt_From_long, 0, 0, 1); if (unlikely(!__pyx_t_1)) __PYX_ERR(1, 12, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_1);
  if (!(likely(PyByteArray_CheckExact(__pyx_t_1))||((__pyx_t_1) == Py_None)||(PyErr_Format(PyExc_TypeError, "Expected %.16s, got %.200s", "bytearray", Py_TYPE(__pyx_t_1)->tp_name), 0))) __PYX_ERR(1, 12, __pyx_L1_error)
  __Pyx_GIVEREF(__pyx_t_1);
  __Pyx_GOTREF(__pyx_v___pyx_result->__pyx_base.buffer);
  __Pyx_DECREF(__pyx_v___pyx_result->__pyx_base.buffer);
  __pyx_v___pyx_result->__pyx_base.buffer = ((PyObject*)__pyx_t_1);
  __pyx_t_1 = 0;
  if (unlikely(__pyx_v___pyx_state == Py_None)) {
    PyErr_SetString(PyExc_TypeError, "'NoneType' object is not subscriptable");
    __PYX_ERR(1, 12, __pyx_L1_error)
  }
  __pyx_t_1 = __Pyx_GetItemInt_Tuple(__pyx_v___pyx_state, 1, long, 1, __Pyx_PyInt_From_long, 0, 0, 1); if (unlikely(!__pyx_t_1)) __PYX_ERR(1, 12, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_1);
  __pyx_t_2 = __Pyx_PyIndex_AsSsize_t(__pyx_t_1); if (unlikely((__pyx_t_2 == (Py_ssize_t)-1) && PyErr_Occurred())) __PYX_ERR(1, 12, __pyx_L1_error)
  __Pyx_DECREF(__pyx_t_1); __pyx_t_1 = 0;
  __pyx_v___pyx_result->__pyx_base.current_buffer_size = __pyx_t_2;
  if (unlikely(__pyx_v___pyx_state == Py_None)) {
    PyErr_SetString(PyExc_TypeError, "'NoneType' object is not subscriptable");
    __PYX_ERR(1, 12, __pyx_L1_error)
  }
  __pyx_t_1 = __Pyx_GetItemInt_Tuple(__pyx_v___pyx_state, 2, long, 1, __Pyx_PyInt_From_long, 0, 0, 1); if (unlikely(!__pyx_t_1)) __PYX_ERR(1, 12, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_1);
  __pyx_t_2 = __Pyx_PyIndex_AsSsize_t(__pyx_t_1); if (unlikely((__pyx_t_2 == (Py_ssize_t)-1) && PyErr_Occurred())) __PYX_ERR(1, 12, __pyx_L1_error)
  __Pyx_DECREF(__pyx_t_1); __pyx_t_1 = 0;
  __pyx_v___pyx_result->__pyx_base.position = __pyx_t_2;
  if (unlikely(__pyx_v___pyx_state == Py_None)) {
    PyErr_SetString(PyExc_TypeError, "'NoneType' object is not subscriptable");
    __PYX_ERR(1, 12, __pyx_L1_error)
  }
  __pyx_t_1 = __Pyx_GetItemInt_Tuple(__pyx_v___pyx_state, 3, long, 1, __Pyx_PyInt_From_long, 0, 0, 1); if (unlikely(!__pyx_t_1)) __PYX_ERR(1, 12, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_1);
  __Pyx_GIVEREF(__pyx_t_1);
  __Pyx_GOTREF(__pyx_v___pyx_result->sock);
  __Pyx_DECREF(__pyx_v___pyx_result->sock);
  __pyx_v___pyx_result->sock = __pyx_t_1;
  __pyx_t_1 = 0;
  if (unlikely(__pyx_v___pyx_state == Py_None)) {
    PyErr_SetString(PyExc_TypeError, "object of type 'NoneType' has no len()");
    __PYX_ERR(1, 13, __pyx_L1_error)
  }
  __pyx_t_2 = PyTuple_GET_SIZE(__pyx_v___pyx_state); if (unlikely(__pyx_t_2 == ((Py_ssize_t)-1))) __PYX_ERR(1, 13, __pyx_L1_error)
  __pyx_t_4 = ((__pyx_t_2 > 4) != 0);
  if (__pyx_t_4) {
  } else {
    __pyx_t_3 = __pyx_t_4;
    goto __pyx_L4_bool_binop_done;
  }
  __pyx_t_4 = __Pyx_HasAttr(((PyObject *)__pyx_v___pyx_result), __pyx_n_s_dict); if (unlikely(__pyx_t_4 == ((int)-1))) __PYX_ERR(1, 13, __pyx_L1_error)
  __pyx_t_5 = (__pyx_t_4 != 0);
  __pyx_t_3 = __pyx_t_5;
  __pyx_L4_bool_binop_done:;
  if (__pyx_t_3) {
    __pyx_t_6 = __Pyx_PyObject_GetAttrStr(((PyObject *)__pyx_v___pyx_result), __pyx_n_s_dict); if (unlikely(!__pyx_t_6)) __PYX_ERR(1, 14, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_6);
    __pyx_t_7 = __Pyx_PyObject_GetAttrStr(__pyx_t_6, __pyx_n_s_update); if (unlikely(!__pyx_t_7)) __PYX_ERR(1, 14, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_7);
    __Pyx_DECREF(__pyx_t_6); __pyx_t_6 = 0;
    if (unlikely(__pyx_v___pyx_state == Py_None)) {
      PyErr_SetString(PyExc_TypeError, "'NoneType' object is not subscriptable");
      __PYX_ERR(1, 14, __pyx_L1_error)
    }
    __pyx_t_6 = __Pyx_GetItemInt_Tuple(__pyx_v___pyx_state, 4, long, 1, __Pyx_PyInt_From_long, 0, 0, 1); if (unlikely(!__pyx_t_6)) __PYX_ERR(1, 14, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_6);
    __pyx_t_8 = NULL;
    if (CYTHON_UNPACK_METHODS && likely(PyMethod_Check(__pyx_t_7))) {
      __pyx_t_8 = PyMethod_GET_SELF(__pyx_t_7);
      if (likely(__pyx_t_8)) {
        PyObject* function = PyMethod_GET_FUNCTION(__pyx_t_7);
        __Pyx_INCREF(__pyx_t_8);
        __Pyx_INCREF(function);
        __Pyx_DECREF_SET(__pyx_t_7, function);
      }
    }
    __pyx_t_1 = (__pyx_t_8) ? __Pyx_PyObject_Call2Args(__pyx_t_7, __pyx_t_8, __pyx_t_6) : __Pyx_PyObject_CallOneArg(__pyx_t_7, __pyx_t_6);
    __Pyx_XDECREF(__pyx_t_8); __pyx_t_8 = 0;
    __Pyx_DECREF(__pyx_t_6); __pyx_t_6 = 0;
    if (unlikely(!__pyx_t_1)) __PYX_ERR(1, 14, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_1);
    __Pyx_DECREF(__pyx_t_7); __pyx_t_7 = 0;
    __Pyx_DECREF(__pyx_t_1); __pyx_t_1 = 0;
  }
  __pyx_r = Py_None; __Pyx_INCREF(Py_None);
  goto __pyx_L0;
  __pyx_L1_error:;
  __Pyx_XDECREF(__pyx_t_1);
  __Pyx_XDECREF(__pyx_t_6);
  __Pyx_XDECREF(__pyx_t_7);
  __Pyx_XDECREF(__pyx_t_8);
  __Pyx_AddTraceback("clickhouse_driver.bufferedreader.__pyx_unpickle_BufferedSocketReader__set_state", __pyx_clineno, __pyx_lineno, __pyx_filename);
  __pyx_r = 0;
  __pyx_L0:;
  __Pyx_XGIVEREF(__pyx_r);
  __Pyx_RefNannyFinishContext();
  return __pyx_r;
}