ast_type_reduce(PyObject *self, PyObject *unused)
{
    PyObject *res;
    _Py_IDENTIFIER(__dict__);
    PyObject *dict = _PyObject_GetAttrId(self, &PyId___dict__);
    if (dict == NULL) {
        if (PyErr_ExceptionMatches(PyExc_AttributeError))
            PyErr_Clear();
        else
            return NULL;
    }
    if (dict) {
        res = Py_BuildValue("O()O", Py_TYPE(self), dict);
        Py_DECREF(dict);
        return res;
    }
    return Py_BuildValue("O()", Py_TYPE(self));
}