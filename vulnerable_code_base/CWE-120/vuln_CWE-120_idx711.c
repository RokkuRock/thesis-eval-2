static PyObject *__pyx_pf_17clickhouse_driver_14bufferedreader_14BufferedReader_8read_strings(struct __pyx_obj_17clickhouse_driver_14bufferedreader_BufferedReader *__pyx_v_self, Py_ssize_t __pyx_v_n_items, PyObject *__pyx_v_encoding) {
  PyObject *__pyx_v_items = NULL;
  Py_ssize_t __pyx_v_i;
  char *__pyx_v_buffer_ptr;
  Py_ssize_t __pyx_v_right;
  Py_ssize_t __pyx_v_size;
  Py_ssize_t __pyx_v_shift;
  Py_ssize_t __pyx_v_bytes_read;
  unsigned char __pyx_v_b;
  char *__pyx_v_c_string;
  Py_ssize_t __pyx_v_c_string_size;
  char *__pyx_v_c_encoding;
  PyObject *__pyx_v_rv = 0;
  PyObject *__pyx_r = NULL;
  __Pyx_RefNannyDeclarations
  PyObject *__pyx_t_1 = NULL;
  int __pyx_t_2;
  PyObject *__pyx_t_3 = NULL;
  PyObject *__pyx_t_4 = NULL;
  char *__pyx_t_5;
  Py_ssize_t __pyx_t_6;
  Py_ssize_t __pyx_t_7;
  Py_ssize_t __pyx_t_8;
  Py_ssize_t __pyx_t_9;
  Py_ssize_t __pyx_t_10;
  Py_ssize_t __pyx_t_11;
  PyObject *__pyx_t_12 = NULL;
  PyObject *__pyx_t_13 = NULL;
  PyObject *__pyx_t_14 = NULL;
  int __pyx_t_15;
  PyObject *__pyx_t_16 = NULL;
  __Pyx_RefNannySetupContext("read_strings", 0);
  __Pyx_INCREF(__pyx_v_encoding);
  __pyx_t_1 = PyTuple_New(__pyx_v_n_items); if (unlikely(!__pyx_t_1)) __PYX_ERR(0, 67, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_1);
  __pyx_v_items = ((PyObject*)__pyx_t_1);
  __pyx_t_1 = 0;
  __pyx_t_1 = __pyx_v_self->buffer;
  __Pyx_INCREF(__pyx_t_1);
  __pyx_v_buffer_ptr = PyByteArray_AsString(__pyx_t_1);
  __Pyx_DECREF(__pyx_t_1); __pyx_t_1 = 0;
  __pyx_v_c_string = NULL;
  __pyx_v_c_string_size = 0x400;
  __pyx_v_c_encoding = NULL;
  __pyx_t_2 = __Pyx_PyObject_IsTrue(__pyx_v_encoding); if (unlikely(__pyx_t_2 < 0)) __PYX_ERR(0, 81, __pyx_L1_error)
  if (__pyx_t_2) {
    __pyx_t_3 = __Pyx_PyObject_GetAttrStr(__pyx_v_encoding, __pyx_n_s_encode); if (unlikely(!__pyx_t_3)) __PYX_ERR(0, 82, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_3);
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
    __pyx_t_1 = (__pyx_t_4) ? __Pyx_PyObject_Call2Args(__pyx_t_3, __pyx_t_4, __pyx_kp_u_utf_8) : __Pyx_PyObject_CallOneArg(__pyx_t_3, __pyx_kp_u_utf_8);
    __Pyx_XDECREF(__pyx_t_4); __pyx_t_4 = 0;
    if (unlikely(!__pyx_t_1)) __PYX_ERR(0, 82, __pyx_L1_error)
    __Pyx_GOTREF(__pyx_t_1);
    __Pyx_DECREF(__pyx_t_3); __pyx_t_3 = 0;
    __Pyx_DECREF_SET(__pyx_v_encoding, __pyx_t_1);
    __pyx_t_1 = 0;
    __pyx_t_5 = __Pyx_PyObject_AsWritableString(__pyx_v_encoding); if (unlikely((!__pyx_t_5) && PyErr_Occurred())) __PYX_ERR(0, 83, __pyx_L1_error)
    __pyx_v_c_encoding = __pyx_t_5;
  }
  __pyx_t_1 = __Pyx_PyObject_CallNoArg(__pyx_builtin_object); if (unlikely(!__pyx_t_1)) __PYX_ERR(0, 85, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_t_1);
  __pyx_v_rv = __pyx_t_1;
  __pyx_t_1 = 0;
  __pyx_t_2 = (__pyx_v_c_encoding != 0);
  if (__pyx_t_2) {
    __pyx_v_c_string = ((char *)PyMem_Realloc(NULL, __pyx_v_c_string_size));
  }
  __pyx_t_6 = __pyx_v_n_items;
  __pyx_t_7 = __pyx_t_6;
  for (__pyx_t_8 = 0; __pyx_t_8 < __pyx_t_7; __pyx_t_8+=1) {
    __pyx_v_i = __pyx_t_8;
    __pyx_v_shift = 0;
    __pyx_v_size = 0;
    while (1) {
      __pyx_t_2 = ((__pyx_v_self->position == __pyx_v_self->current_buffer_size) != 0);
      if (__pyx_t_2) {
        __pyx_t_3 = __Pyx_PyObject_GetAttrStr(((PyObject *)__pyx_v_self), __pyx_n_s_read_into_buffer); if (unlikely(!__pyx_t_3)) __PYX_ERR(0, 96, __pyx_L1_error)
        __Pyx_GOTREF(__pyx_t_3);
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
        __pyx_t_1 = (__pyx_t_4) ? __Pyx_PyObject_CallOneArg(__pyx_t_3, __pyx_t_4) : __Pyx_PyObject_CallNoArg(__pyx_t_3);
        __Pyx_XDECREF(__pyx_t_4); __pyx_t_4 = 0;
        if (unlikely(!__pyx_t_1)) __PYX_ERR(0, 96, __pyx_L1_error)
        __Pyx_GOTREF(__pyx_t_1);
        __Pyx_DECREF(__pyx_t_3); __pyx_t_3 = 0;
        __Pyx_DECREF(__pyx_t_1); __pyx_t_1 = 0;
        __pyx_t_1 = __pyx_v_self->buffer;
        __Pyx_INCREF(__pyx_t_1);
        __pyx_v_buffer_ptr = PyByteArray_AsString(__pyx_t_1);
        __Pyx_DECREF(__pyx_t_1); __pyx_t_1 = 0;
        __pyx_v_self->position = 0;
      }
      __pyx_v_b = (__pyx_v_buffer_ptr[__pyx_v_self->position]);
      __pyx_v_self->position = (__pyx_v_self->position + 1);
      __pyx_v_size = (__pyx_v_size | ((__pyx_v_b & 0x7f) << __pyx_v_shift));
      __pyx_t_2 = ((__pyx_v_b < 0x80) != 0);
      if (__pyx_t_2) {
        goto __pyx_L8_break;
      }
      __pyx_v_shift = (__pyx_v_shift + 7);
    }
    __pyx_L8_break:;
    __pyx_v_right = (__pyx_v_self->position + __pyx_v_size);
    __pyx_t_2 = (__pyx_v_c_encoding != 0);
    if (__pyx_t_2) {
      __pyx_t_2 = (((__pyx_v_size + 1) > __pyx_v_c_string_size) != 0);
      if (__pyx_t_2) {
        __pyx_v_c_string_size = (__pyx_v_size + 1);
        __pyx_v_c_string = ((char *)PyMem_Realloc(__pyx_v_c_string, __pyx_v_c_string_size));
        __pyx_t_2 = ((__pyx_v_c_string == NULL) != 0);
        if (unlikely(__pyx_t_2)) {
          PyErr_NoMemory(); __PYX_ERR(0, 117, __pyx_L1_error)
        }
      }
      (__pyx_v_c_string[__pyx_v_size]) = 0;
      __pyx_v_bytes_read = 0;
    }
    __pyx_t_2 = ((__pyx_v_right > __pyx_v_self->current_buffer_size) != 0);
    if (__pyx_t_2) {
      __pyx_t_2 = (__pyx_v_c_encoding != 0);
      if (__pyx_t_2) {
        (void)(memcpy((&(__pyx_v_c_string[__pyx_v_bytes_read])), (&(__pyx_v_buffer_ptr[__pyx_v_self->position])), (__pyx_v_self->current_buffer_size - __pyx_v_self->position)));
        goto __pyx_L15;
      }
        {
        __pyx_t_1 = PyBytes_FromStringAndSize((&(__pyx_v_buffer_ptr[__pyx_v_self->position])), (__pyx_v_self->current_buffer_size - __pyx_v_self->position)); if (unlikely(!__pyx_t_1)) __PYX_ERR(0, 129, __pyx_L1_error)
        __Pyx_GOTREF(__pyx_t_1);
        __Pyx_DECREF_SET(__pyx_v_rv, __pyx_t_1);
        __pyx_t_1 = 0;
      }
      __pyx_L15:;
      __pyx_v_bytes_read = (__pyx_v_self->current_buffer_size - __pyx_v_self->position);
      while (1) {
        __pyx_t_2 = ((__pyx_v_bytes_read != __pyx_v_size) != 0);
        if (!__pyx_t_2) break;
        __pyx_v_self->position = (__pyx_v_size - __pyx_v_bytes_read);
        __pyx_t_3 = __Pyx_PyObject_GetAttrStr(((PyObject *)__pyx_v_self), __pyx_n_s_read_into_buffer); if (unlikely(!__pyx_t_3)) __PYX_ERR(0, 139, __pyx_L1_error)
        __Pyx_GOTREF(__pyx_t_3);
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
        __pyx_t_1 = (__pyx_t_4) ? __Pyx_PyObject_CallOneArg(__pyx_t_3, __pyx_t_4) : __Pyx_PyObject_CallNoArg(__pyx_t_3);
        __Pyx_XDECREF(__pyx_t_4); __pyx_t_4 = 0;
        if (unlikely(!__pyx_t_1)) __PYX_ERR(0, 139, __pyx_L1_error)
        __Pyx_GOTREF(__pyx_t_1);
        __Pyx_DECREF(__pyx_t_3); __pyx_t_3 = 0;
        __Pyx_DECREF(__pyx_t_1); __pyx_t_1 = 0;
        __pyx_t_1 = __pyx_v_self->buffer;
        __Pyx_INCREF(__pyx_t_1);
        __pyx_v_buffer_ptr = PyByteArray_AsString(__pyx_t_1);
        __Pyx_DECREF(__pyx_t_1); __pyx_t_1 = 0;
        __pyx_t_9 = __pyx_v_self->current_buffer_size;
        __pyx_t_10 = __pyx_v_self->position;
        if (((__pyx_t_9 < __pyx_t_10) != 0)) {
          __pyx_t_11 = __pyx_t_9;
        } else {
          __pyx_t_11 = __pyx_t_10;
        }
        __pyx_v_self->position = __pyx_t_11;
        __pyx_t_2 = (__pyx_v_c_encoding != 0);
        if (__pyx_t_2) {
          (void)(memcpy((&(__pyx_v_c_string[__pyx_v_bytes_read])), __pyx_v_buffer_ptr, __pyx_v_self->position));
          goto __pyx_L18;
        }
          {
          __pyx_t_1 = PyBytes_FromStringAndSize(__pyx_v_buffer_ptr, __pyx_v_self->position); if (unlikely(!__pyx_t_1)) __PYX_ERR(0, 151, __pyx_L1_error)
          __Pyx_GOTREF(__pyx_t_1);
          __pyx_t_3 = PyNumber_InPlaceAdd(__pyx_v_rv, __pyx_t_1); if (unlikely(!__pyx_t_3)) __PYX_ERR(0, 151, __pyx_L1_error)
          __Pyx_GOTREF(__pyx_t_3);
          __Pyx_DECREF(__pyx_t_1); __pyx_t_1 = 0;
          __Pyx_DECREF_SET(__pyx_v_rv, __pyx_t_3);
          __pyx_t_3 = 0;
        }
        __pyx_L18:;
        __pyx_v_bytes_read = (__pyx_v_bytes_read + __pyx_v_self->position);
      }
      goto __pyx_L14;
    }
      {
      __pyx_t_2 = (__pyx_v_c_encoding != 0);
      if (__pyx_t_2) {
        (void)(memcpy(__pyx_v_c_string, (&(__pyx_v_buffer_ptr[__pyx_v_self->position])), __pyx_v_size));
        goto __pyx_L19;
      }
        {
        __pyx_t_3 = PyBytes_FromStringAndSize((&(__pyx_v_buffer_ptr[__pyx_v_self->position])), __pyx_v_size); if (unlikely(!__pyx_t_3)) __PYX_ERR(0, 160, __pyx_L1_error)
        __Pyx_GOTREF(__pyx_t_3);
        __Pyx_DECREF_SET(__pyx_v_rv, __pyx_t_3);
        __pyx_t_3 = 0;
      }
      __pyx_L19:;
      __pyx_v_self->position = __pyx_v_right;
    }
    __pyx_L14:;
    __pyx_t_2 = (__pyx_v_c_encoding != 0);
    if (__pyx_t_2) {
      {
        __Pyx_PyThreadState_declare
        __Pyx_PyThreadState_assign
        __Pyx_ExceptionSave(&__pyx_t_12, &__pyx_t_13, &__pyx_t_14);
        __Pyx_XGOTREF(__pyx_t_12);
        __Pyx_XGOTREF(__pyx_t_13);
        __Pyx_XGOTREF(__pyx_t_14);
          {
          __pyx_t_3 = __Pyx_decode_c_string(__pyx_v_c_string, 0, __pyx_v_size, __pyx_v_c_encoding, NULL, NULL); if (unlikely(!__pyx_t_3)) __PYX_ERR(0, 167, __pyx_L21_error)
          __Pyx_GOTREF(__pyx_t_3);
          __Pyx_DECREF_SET(__pyx_v_rv, __pyx_t_3);
          __pyx_t_3 = 0;
        }
        __Pyx_XDECREF(__pyx_t_12); __pyx_t_12 = 0;
        __Pyx_XDECREF(__pyx_t_13); __pyx_t_13 = 0;
        __Pyx_XDECREF(__pyx_t_14); __pyx_t_14 = 0;
        goto __pyx_L28_try_end;
        __pyx_L21_error:;
        __Pyx_XDECREF(__pyx_t_1); __pyx_t_1 = 0;
        __Pyx_XDECREF(__pyx_t_3); __pyx_t_3 = 0;
        __Pyx_XDECREF(__pyx_t_4); __pyx_t_4 = 0;
        __pyx_t_15 = __Pyx_PyErr_ExceptionMatches(__pyx_builtin_UnicodeDecodeError);
        if (__pyx_t_15) {
          __Pyx_AddTraceback("clickhouse_driver.bufferedreader.BufferedReader.read_strings", __pyx_clineno, __pyx_lineno, __pyx_filename);
          if (__Pyx_GetException(&__pyx_t_3, &__pyx_t_1, &__pyx_t_4) < 0) __PYX_ERR(0, 168, __pyx_L23_except_error)
          __Pyx_GOTREF(__pyx_t_3);
          __Pyx_GOTREF(__pyx_t_1);
          __Pyx_GOTREF(__pyx_t_4);
          __pyx_t_16 = PyBytes_FromStringAndSize(__pyx_v_c_string, __pyx_v_size); if (unlikely(!__pyx_t_16)) __PYX_ERR(0, 169, __pyx_L23_except_error)
          __Pyx_GOTREF(__pyx_t_16);
          __Pyx_DECREF_SET(__pyx_v_rv, __pyx_t_16);
          __pyx_t_16 = 0;
          __Pyx_XDECREF(__pyx_t_3); __pyx_t_3 = 0;
          __Pyx_XDECREF(__pyx_t_1); __pyx_t_1 = 0;
          __Pyx_XDECREF(__pyx_t_4); __pyx_t_4 = 0;
          goto __pyx_L22_exception_handled;
        }
        goto __pyx_L23_except_error;
        __pyx_L23_except_error:;
        __Pyx_XGIVEREF(__pyx_t_12);
        __Pyx_XGIVEREF(__pyx_t_13);
        __Pyx_XGIVEREF(__pyx_t_14);
        __Pyx_ExceptionReset(__pyx_t_12, __pyx_t_13, __pyx_t_14);
        goto __pyx_L1_error;
        __pyx_L22_exception_handled:;
        __Pyx_XGIVEREF(__pyx_t_12);
        __Pyx_XGIVEREF(__pyx_t_13);
        __Pyx_XGIVEREF(__pyx_t_14);
        __Pyx_ExceptionReset(__pyx_t_12, __pyx_t_13, __pyx_t_14);
        __pyx_L28_try_end:;
      }
    }
    Py_INCREF(__pyx_v_rv);
    PyTuple_SET_ITEM(__pyx_v_items, __pyx_v_i, __pyx_v_rv);
  }
  __pyx_t_2 = (__pyx_v_c_string != 0);
  if (__pyx_t_2) {
    PyMem_Free(__pyx_v_c_string);
  }
  __Pyx_XDECREF(__pyx_r);
  __Pyx_INCREF(__pyx_v_items);
  __pyx_r = __pyx_v_items;
  goto __pyx_L0;
  __pyx_L1_error:;
  __Pyx_XDECREF(__pyx_t_1);
  __Pyx_XDECREF(__pyx_t_3);
  __Pyx_XDECREF(__pyx_t_4);
  __Pyx_XDECREF(__pyx_t_16);
  __Pyx_AddTraceback("clickhouse_driver.bufferedreader.BufferedReader.read_strings", __pyx_clineno, __pyx_lineno, __pyx_filename);
  __pyx_r = NULL;
  __pyx_L0:;
  __Pyx_XDECREF(__pyx_v_items);
  __Pyx_XDECREF(__pyx_v_rv);
  __Pyx_XDECREF(__pyx_v_encoding);
  __Pyx_XGIVEREF(__pyx_r);
  __Pyx_RefNannyFinishContext();
  return __pyx_r;
}