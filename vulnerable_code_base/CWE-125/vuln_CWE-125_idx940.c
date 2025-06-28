static int add_attributes(PyTypeObject* type, char**attrs, int num_fields)
{
    int i, result;
    _Py_IDENTIFIER(_attributes);
    PyObject *s, *l = PyTuple_New(num_fields);
    if (!l)
        return 0;
    for (i = 0; i < num_fields; i++) {
        s = PyUnicode_FromString(attrs[i]);
        if (!s) {
            Py_DECREF(l);
            return 0;
        }
        PyTuple_SET_ITEM(l, i, s);
    }
    result = _PyObject_SetAttrId((PyObject*)type, &PyId__attributes, l) >= 0;
    Py_DECREF(l);
    return result;
}