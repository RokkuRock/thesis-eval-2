static PyObject *__pyx_f_17clickhouse_driver_14bufferedwriter_14BufferedWriter_write(struct __pyx_obj_17clickhouse_driver_14bufferedwriter_BufferedWriter *__pyx_v_self, PyObject *__pyx_v_data, int __pyx_skip_dispatch) {
  Py_ssize_t __pyx_v_written;
  Py_ssize_t __pyx_v_size;
  Py_ssize_t __pyx_v_data_len;
  char *__pyx_v_c_data;
  PyObject *__pyx_r = NULL;
  __Pyx_RefNannyDeclarations
  PyObject *__pyx_t_1 = NULL;
  PyObject *__pyx_t_2 = NULL;
  PyObject *__pyx_t_3 = NULL;
  PyObject *__pyx_t_4 = NULL;
  Py_ssize_t __pyx_t_5;
  char *__pyx_t_6;
  int __pyx_t_7;
  Py_ssize_t __pyx_t_8;
  Py_ssize_t __pyx_t_9;
  __Pyx_RefNannySetupContext("write", 0);
  if (unlikely(__pyx_skip_dispatch)) ;
  else if (unlikely((Py_TYPE(((PyObject *)__pyx_v_self))->tp_dictoffset != 0) || (Py_TYPE(((PyObject *)__pyx_v_self))->tp_flags & (Py_TPFLAGS_IS_ABSTRACT | Py_TPFLAGS_HEAPTYPE)))) {
    #if CYTHON_USE_DICT_VERSIONS && CYTHON_USE_PYTYPE_LOOKUP && CYTHON_USE_TYPE_SLOTS
    static PY_UINT64_T __pyx_tp_dict_version = __PYX_DICT_VERSION_INIT, __pyx_obj_dict_version = __PYX_DICT_VERSION_INIT;
    if (unlikely(!__Pyx_object_dict_version_matches(((PyObject *)__pyx_v_self), __pyx_tp_dict_version, __pyx_obj_dict_version))) {
      PY_UINT64_T __pyx_type_dict_guard = __Pyx_get_tp_dict_version(((PyObject *)__pyx_v_self));
      #endif
      __pyx_t_1 = __Pyx_PyObject_GetAttrStr(((PyObject *)__pyx_v_self), __pyx_n_s_write); if (unlikely(!__pyx_t_1)) __PYX_ERR(0, 28, __pyx_L1_error)
      __Pyx_GOTREF(__pyx_t_1);
      if (!PyCFunction_Check(__pyx_t_1) || (PyCFunction_GET_FUNCTION(__pyx_t_1) != (PyCFunction)(void*)__pyx_pw_17clickhouse_driver_14bufferedwriter_14BufferedWriter_7write)) {
        __Pyx_XDECREF(__pyx_r);
        __Pyx_INCREF(__pyx_t_1);
        __pyx_t_3 = __pyx_t_1; __pyx_t_4 = NULL;
        if (CYTHON_UNPACK_METHODS && unlikely(PyMethod_Check(__pyx_t_3))) {
          __pyx_t_4 = PyMethod_GET_SELF(__pyx_t_3);
          if (likely(__pyx_t_4)) {
            PyObject* function = PyMethod_GET_FUNCTION(__pyx_t_3);
            __Pyx_INCREF(__pyx_t_4);
            __Pyx_INCREF(function);
            __Pyx_DECREF_SET(__pyx_t_3, function);
          }
        }
        __pyx_t_2 = (__pyx_t_4) ? __Pyx_PyObject_Call2Args(__pyx_t_3, __pyx_t_4, __pyx_v_data) : __Pyx_PyObject_CallOneArg(__pyx_t_3, __pyx_v_data);
        __Pyx_XDECREF(__pyx_t_4); __pyx_t_4 = 0;
        if (unlikely(!__pyx_t_2)) __PYX_ERR(0, 28, __pyx_L1_error)
        __Pyx_GOTREF(__pyx_t_2);
        __Pyx_DECREF(__pyx_t_3); __pyx_t_3 = 0;
        __pyx_r = __pyx_t_2;
        __pyx_t_2 = 0;
        __Pyx_DECREF(__pyx_t_1); __pyx_t_1 = 0;
        goto __pyx_L0;
      }
      #if CYTHON_USE_DICT_VERSIONS && CYTHON_USE_PYTYPE_LOOKUP && CYTHON_USE_TYPE_SLOTS
      __pyx_tp_dict_version = __Pyx_get_tp_dict_version(((PyObject *)__pyx_v_self));
      __pyx_obj_dict_version = __Pyx_get_object_dict_version(((PyObject *)__pyx_v_self));
      if (unlikely(__pyx_type_dict_guard != __pyx_tp_dict_version)) {
        __pyx_tp_dict_version = __pyx_obj_dict_version = __PYX_DICT_VERSION_INIT;
      }
      #endif
      __Pyx_DECREF(__pyx_t_1); __pyx_t_1 = 0;
      #if CYTHON_USE_DICT_VERSIONS && CYTHON_USE_PYTYPE_LOOKUP && CYTHON_USE_TYPE_SLOTS
    }
    #endif
  }
  __pyx_v_written = 0;
  __pyx_t_5 = PyObject_Length(__pyx_v_data); if (unlikely(__pyx_t_5 == ((Py_ssize_t)-1))) __PYX_ERR(0, 31, __pyx_L1_error)
  __pyx_v_data_len = __pyx_t_5;
  __pyx_t_6 = PyBytes_AsString(__pyx_v_data); if (unlikely(__pyx_t_6 == ((char *)NULL))) __PYX_ERR(0, 34, __pyx_L1_error)
  __pyx_v_c_data = __pyx_t_6;
  while (1) {
    __pyx_t_7 = ((__pyx_v_written < __pyx_v_data_len) != 0);
    if (!__pyx_t_7) break;
    __pyx_t_5 = (__pyx_v_self->buffer_size - __pyx_v_self->position);
    __pyx_t_8 = (__pyx_v_data_len - __pyx_v_written);
    if (((__pyx_t_5 < __pyx_t_8) != 0)) {
      __pyx_t_9 = __pyx_t_5;
    } else {
      __pyx_t_9 = __pyx_t_8;
    }
    __pyx_v_size = __pyx_t_9;
    (void)(memcpy((&(__pyx_v_self->buffer[__pyx_v_self->position])), (&(__pyx_v_c_data[__pyx_v_written])), __pyx_v_size));
    __pyx_t_7 = ((__pyx_v_self->position == __pyx_v_self->buffer_size) != 0);
    if (__pyx_t_7) {
      __pyx_t_1 = ((struct __pyx_vtabstruct_17clickhouse_driver_14bufferedwriter_BufferedWriter *)__pyx_v_self->__pyx_vtab)->write_into_stream(__pyx_v_self, 0); if (unlikely(!__pyx_t_1)) __PYX_ERR(0, 41, __pyx_L1_error)
      __Pyx_GOTREF(__pyx_t_1);
      __Pyx_DECREF(__pyx_t_1); __pyx_t_1 = 0;
    }
    __pyx_v_self->position = (__pyx_v_self->position + __pyx_v_size);
    __pyx_v_written = (__pyx_v_written + __pyx_v_size);
  }
  __pyx_r = Py_None; __Pyx_INCREF(Py_None);
  goto __pyx_L0;
  __pyx_L1_error:;
  __Pyx_XDECREF(__pyx_t_1);
  __Pyx_XDECREF(__pyx_t_2);
  __Pyx_XDECREF(__pyx_t_3);
  __Pyx_XDECREF(__pyx_t_4);
  __Pyx_AddTraceback("clickhouse_driver.bufferedwriter.BufferedWriter.write", __pyx_clineno, __pyx_lineno, __pyx_filename);
  __pyx_r = 0;
  __pyx_L0:;
  __Pyx_XGIVEREF(__pyx_r);
  __Pyx_RefNannyFinishContext();
  return __pyx_r;
}