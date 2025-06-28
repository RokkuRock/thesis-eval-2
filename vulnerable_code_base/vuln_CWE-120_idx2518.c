static CYTHON_SMALL_CODE int __Pyx_InitCachedConstants(void) {
  __Pyx_RefNannyDeclarations
  __Pyx_RefNannySetupContext("__Pyx_InitCachedConstants", 0);
  __pyx_tuple_ = PyTuple_Pack(5, __pyx_n_s_number, __pyx_n_s_buf, __pyx_n_s_i, __pyx_n_s_towrite, __pyx_n_s_num_buf); if (unlikely(!__pyx_tuple_)) __PYX_ERR(0, 4, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_tuple_);
  __Pyx_GIVEREF(__pyx_tuple_);
  __pyx_codeobj__2 = (PyObject*)__Pyx_PyCode_New(2, 0, 5, 0, CO_OPTIMIZED|CO_NEWLOCALS, __pyx_empty_bytes, __pyx_empty_tuple, __pyx_empty_tuple, __pyx_tuple_, __pyx_empty_tuple, __pyx_empty_tuple, __pyx_kp_s_clickhouse_driver_varint_pyx, __pyx_n_s_write_varint, 4, __pyx_empty_bytes); if (unlikely(!__pyx_codeobj__2)) __PYX_ERR(0, 4, __pyx_L1_error)
  __pyx_tuple__3 = PyTuple_Pack(5, __pyx_n_s_f, __pyx_n_s_shift, __pyx_n_s_result, __pyx_n_s_i, __pyx_n_s_read_one); if (unlikely(!__pyx_tuple__3)) __PYX_ERR(0, 29, __pyx_L1_error)
  __Pyx_GOTREF(__pyx_tuple__3);
  __Pyx_GIVEREF(__pyx_tuple__3);
  __pyx_codeobj__4 = (PyObject*)__Pyx_PyCode_New(1, 0, 5, 0, CO_OPTIMIZED|CO_NEWLOCALS, __pyx_empty_bytes, __pyx_empty_tuple, __pyx_empty_tuple, __pyx_tuple__3, __pyx_empty_tuple, __pyx_empty_tuple, __pyx_kp_s_clickhouse_driver_varint_pyx, __pyx_n_s_read_varint, 29, __pyx_empty_bytes); if (unlikely(!__pyx_codeobj__4)) __PYX_ERR(0, 29, __pyx_L1_error)
  __Pyx_RefNannyFinishContext();
  return 0;
  __pyx_L1_error:;
  __Pyx_RefNannyFinishContext();
  return -1;
}