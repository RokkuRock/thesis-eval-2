static int exists_not_none(PyObject *obj, _Py_Identifier *id)
{
    int isnone;
    PyObject *attr = _PyObject_GetAttrId(obj, id);
    if (!attr) {
        PyErr_Clear();
        return 0;
    }
    isnone = attr == Py_None;
    Py_DECREF(attr);
    return !isnone;
}