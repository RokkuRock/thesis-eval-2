obj2ast_keyword(PyObject* obj, keyword_ty* out, PyArena* arena)
{
    PyObject* tmp = NULL;
    identifier arg;
    expr_ty value;
    if (exists_not_none(obj, &PyId_arg)) {
        int res;
        tmp = _PyObject_GetAttrId(obj, &PyId_arg);
        if (tmp == NULL) goto failed;
        res = obj2ast_identifier(tmp, &arg, arena);
        if (res != 0) goto failed;
        Py_CLEAR(tmp);
    } else {
        arg = NULL;
    }
    if (_PyObject_HasAttrId(obj, &PyId_value)) {
        int res;
        tmp = _PyObject_GetAttrId(obj, &PyId_value);
        if (tmp == NULL) goto failed;
        res = obj2ast_expr(tmp, &value, arena);
        if (res != 0) goto failed;
        Py_CLEAR(tmp);
    } else {
        PyErr_SetString(PyExc_TypeError, "required field \"value\" missing from keyword");
        return 1;
    }
    *out = keyword(arg, value, arena);
    return 0;
failed:
    Py_XDECREF(tmp);
    return 1;
}