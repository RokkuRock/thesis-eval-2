ast_type_init(PyObject *self, PyObject *args, PyObject *kw)
{
    _Py_IDENTIFIER(_fields);
    Py_ssize_t i, numfields = 0;
    int res = -1;
    PyObject *key, *value, *fields;
    fields = _PyObject_GetAttrId((PyObject*)Py_TYPE(self), &PyId__fields);
    if (!fields)
        PyErr_Clear();
    if (fields) {
        numfields = PySequence_Size(fields);
        if (numfields == -1)
            goto cleanup;
    }
    res = 0;  
    if (PyTuple_GET_SIZE(args) > 0) {
        if (numfields != PyTuple_GET_SIZE(args)) {
            PyErr_Format(PyExc_TypeError, "%.400s constructor takes %s"
                         "%zd positional argument%s",
                         Py_TYPE(self)->tp_name,
                         numfields == 0 ? "" : "either 0 or ",
                         numfields, numfields == 1 ? "" : "s");
            res = -1;
            goto cleanup;
        }
        for (i = 0; i < PyTuple_GET_SIZE(args); i++) {
            PyObject *name = PySequence_GetItem(fields, i);
            if (!name) {
                res = -1;
                goto cleanup;
            }
            res = PyObject_SetAttr(self, name, PyTuple_GET_ITEM(args, i));
            Py_DECREF(name);
            if (res < 0)
                goto cleanup;
        }
    }
    if (kw) {
        i = 0;   
        while (PyDict_Next(kw, &i, &key, &value)) {
            res = PyObject_SetAttr(self, key, value);
            if (res < 0)
                goto cleanup;
        }
    }
  cleanup:
    Py_XDECREF(fields);
    return res;
}