static PyObject *__pyx_pf_17clickhouse_driver_14bufferedwriter_14BufferedWriter_12__reduce_cython__(struct __pyx_obj_17clickhouse_driver_14bufferedwriter_BufferedWriter *__pyx_v_self) {
  PyObject *__pyx_v_state = 0;
  PyObject *__pyx_v__dict = 0;
  int __pyx_v_use_setstate;
  PyObject *__pyx_r = NULL;
  __Pyx_RefNannyDeclarations
  PyObject *__pyx_t_1 = NULL;
  PyObject *__pyx_t_2 = NULL;
  PyObject *__pyx_t_3 = NULL;
  PyObject *__pyx_t_4 = NULL;
  int __pyx_t_5;
  int __pyx_t_6;
  __Pyx_RefNannySetupContext("__reduce_cython__", 0);
  __pyx_t_1 = __Pyx_PyBytes_FromString(__pyx_v_self->buffer); if (unlikely(!__pyx_t_1)) __PYX_ERR(1, 5, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_1);
  __pyx_t_2 = PyInt_FromSsize_t(__pyx_v_self->buffer_size); if (unlikely(!__pyx_t_2)) __PYX_ERR(1, 5, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_2);
  __pyx_t_3 = PyInt_FromSsize_t(__pyx_v_self->position); if (unlikely(!__pyx_t_3)) __PYX_ERR(1, 5, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_3);
  __pyx_t_4 = PyTuple_New(3); if (unlikely(!__pyx_t_4)) __PYX_ERR(1, 5, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_4);
  __Pyx_GIVEREF(__pyx_t_1);
  PyTuple_SET_ITEM(__pyx_t_4, 0, __pyx_t_1);
  __Pyx_GIVEREF(__pyx_t_2);
  PyTuple_SET_ITEM(__pyx_t_4, 1, __pyx_t_2);
  __Pyx_GIVEREF(__pyx_t_3);
  PyTuple_SET_ITEM(__pyx_t_4, 2, __pyx_t_3);
  __pyx_t_1 = 0;
  __pyx_t_2 = 0;
  __pyx_t_3 = 0;
  __pyx_v_state = ((PyObject*)__pyx_t_4);
  __pyx_t_4 = 0;
  __pyx_t_4 = __Pyx_GetAttr3(((PyObject *)__pyx_v_self), __pyx_n_s_dict, Py_None); if (unlikely(!__pyx_t_4)) __PYX_ERR(1, 6, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_4);
  __pyx_v__dict = __pyx_t_4;
  __pyx_t_4 = 0;
  __pyx_t_5 = (__pyx_v__dict != Py_None);
  __pyx_t_6 = (__pyx_t_5 != 0);
  if (__pyx_t_6) {
    __pyx_t_4 = PyTuple_New(1); if (unlikely(!__pyx_t_4)) __PYX_ERR(1, 8, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_4);
    __Pyx_INCREF(__pyx_v__dict);
    __Pyx_GIVEREF(__pyx_v__dict);
    PyTuple_SET_ITEM(__pyx_t_4, 0, __pyx_v__dict);
    __pyx_t_3 = PyNumber_InPlaceAdd(__pyx_v_state, __pyx_t_4); if (unlikely(!__pyx_t_3)) __PYX_ERR(1, 8, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_3);
    __Pyx_DECREF(__pyx_t_4); __pyx_t_4 = 0;
    __Pyx_DECREF_SET(__pyx_v_state, ((PyObject*)__pyx_t_3));
    __pyx_t_3 = 0;
    __pyx_v_use_setstate = 1;
    goto __pyx_L3;
  }
    {
    __pyx_v_use_setstate = 0;
  }
  __pyx_L3:;
  __pyx_t_6 = (__pyx_v_use_setstate != 0);
  if (__pyx_t_6) {
    __Pyx_XDECREF(__pyx_r);
    __Pyx_GetModuleGlobalName(__pyx_t_3, __pyx_n_s_pyx_unpickle_BufferedWriter); if (unlikely(!__pyx_t_3)) __PYX_ERR(1, 13, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_3);
    __pyx_t_4 = PyTuple_New(3); if (unlikely(!__pyx_t_4)) __PYX_ERR(1, 13, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_4);
    __Pyx_INCREF(((PyObject *)Py_TYPE(((PyObject *)__pyx_v_self))));
    __Pyx_GIVEREF(((PyObject *)Py_TYPE(((PyObject *)__pyx_v_self))));
    PyTuple_SET_ITEM(__pyx_t_4, 0, ((PyObject *)Py_TYPE(((PyObject *)__pyx_v_self))));
    __Pyx_INCREF(__pyx_int_39656716);
    __Pyx_GIVEREF(__pyx_int_39656716);
    PyTuple_SET_ITEM(__pyx_t_4, 1, __pyx_int_39656716);
    __Pyx_INCREF(Py_None);
    __Pyx_GIVEREF(Py_None);
    PyTuple_SET_ITEM(__pyx_t_4, 2, Py_None);
    __pyx_t_2 = PyTuple_New(3); if (unlikely(!__pyx_t_2)) __PYX_ERR(1, 13, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_2);
    __Pyx_GIVEREF(__pyx_t_3);
    PyTuple_SET_ITEM(__pyx_t_2, 0, __pyx_t_3);
    __Pyx_GIVEREF(__pyx_t_4);
    PyTuple_SET_ITEM(__pyx_t_2, 1, __pyx_t_4);
    __Pyx_INCREF(__pyx_v_state);
    __Pyx_GIVEREF(__pyx_v_state);
    PyTuple_SET_ITEM(__pyx_t_2, 2, __pyx_v_state);
    __pyx_t_3 = 0;
    __pyx_t_4 = 0;
    __pyx_r = __pyx_t_2;
    __pyx_t_2 = 0;
    goto __pyx_L0;
  }
    {
    __Pyx_XDECREF(__pyx_r);
    __Pyx_GetModuleGlobalName(__pyx_t_2, __pyx_n_s_pyx_unpickle_BufferedWriter); if (unlikely(!__pyx_t_2)) __PYX_ERR(1, 15, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_2);
    __pyx_t_4 = PyTuple_New(3); if (unlikely(!__pyx_t_4)) __PYX_ERR(1, 15, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_4);
    __Pyx_INCREF(((PyObject *)Py_TYPE(((PyObject *)__pyx_v_self))));
    __Pyx_GIVEREF(((PyObject *)Py_TYPE(((PyObject *)__pyx_v_self))));
    PyTuple_SET_ITEM(__pyx_t_4, 0, ((PyObject *)Py_TYPE(((PyObject *)__pyx_v_self))));
    __Pyx_INCREF(__pyx_int_39656716);
    __Pyx_GIVEREF(__pyx_int_39656716);
    PyTuple_SET_ITEM(__pyx_t_4, 1, __pyx_int_39656716);
    __Pyx_INCREF(__pyx_v_state);
    __Pyx_GIVEREF(__pyx_v_state);
    PyTuple_SET_ITEM(__pyx_t_4, 2, __pyx_v_state);
    __pyx_t_3 = PyTuple_New(2); if (unlikely(!__pyx_t_3)) __PYX_ERR(1, 15, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_3);
    __Pyx_GIVEREF(__pyx_t_2);
    PyTuple_SET_ITEM(__pyx_t_3, 0, __pyx_t_2);
    __Pyx_GIVEREF(__pyx_t_4);
    PyTuple_SET_ITEM(__pyx_t_3, 1, __pyx_t_4);
    __pyx_t_2 = 0;
    __pyx_t_4 = 0;
    __pyx_r = __pyx_t_3;
    __pyx_t_3 = 0;
    goto __pyx_L0;
  }
  __pyx_L1_error:;
  __Pyx_XDECREF(__pyx_t_1);
  __Pyx_XDECREF(__pyx_t_2);
  __Pyx_XDECREF(__pyx_t_3);
  __Pyx_XDECREF(__pyx_t_4);
  __Pyx_AddTraceback("clickhouse_driver.bufferedwriter.BufferedWriter.__reduce_cython__", __pyx_clineno, __pyx_lineno, __pyx_filename);
  __pyx_r = NULL;
  __pyx_L0:;
  __Pyx_XDECREF(__pyx_v_state);
  __Pyx_XDECREF(__pyx_v__dict);
  __Pyx_XGIVEREF(__pyx_r);
  __Pyx_RefNannyFinishContext();
  return __pyx_r;
}