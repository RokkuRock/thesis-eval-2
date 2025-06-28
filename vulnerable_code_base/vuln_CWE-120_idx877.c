static void __pyx_tp_dealloc_17clickhouse_driver_14bufferedwriter_BufferedWriter(PyObject *o) {
  #if CYTHON_USE_TP_FINALIZE
  if (unlikely(PyType_HasFeature(Py_TYPE(o), Py_TPFLAGS_HAVE_FINALIZE) && Py_TYPE(o)->tp_finalize) && (!PyType_IS_GC(Py_TYPE(o)) || !_PyGC_FINALIZED(o))) {
    if (PyObject_CallFinalizerFromDealloc(o)) return;
  }
  #endif
  {
    PyObject *etype, *eval, *etb;
    PyErr_Fetch(&etype, &eval, &etb);
    ++Py_REFCNT(o);
    __pyx_pw_17clickhouse_driver_14bufferedwriter_14BufferedWriter_3__dealloc__(o);
    --Py_REFCNT(o);
    PyErr_Restore(etype, eval, etb);
  }
  (*Py_TYPE(o)->tp_free)(o);
}