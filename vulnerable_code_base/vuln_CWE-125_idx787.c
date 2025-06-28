obj2ast_alias(PyObject* obj, alias_ty* out, PyArena* arena)
{
    PyObject* tmp = NULL;
    identifier name;
    identifier asname;
    if (_PyObject_HasAttrId(obj, &PyId_name)) {
        int res;
        tmp = _PyObject_GetAttrId(obj, &PyId_name);
        if (tmp == NULL) goto failed;
        res = obj2ast_identifier(tmp, &name, arena);
        if (res != 0) goto failed;
        Py_CLEAR(tmp);
    } else {
        PyErr_SetString(PyExc_TypeError, "required field \"name\" missing from alias");
        return 1;
    }
    if (exists_not_none(obj, &PyId_asname)) {
        int res;
        tmp = _PyObject_GetAttrId(obj, &PyId_asname);
        if (tmp == NULL) goto failed;
        res = obj2ast_identifier(tmp, &asname, arena);
        if (res != 0) goto failed;
        Py_CLEAR(tmp);
    } else {
        asname = NULL;
    }
    *out = alias(name, asname, arena);
    return 0;
failed:
    Py_XDECREF(tmp);
    return 1;
}