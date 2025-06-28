obj2ast_withitem(PyObject* obj, withitem_ty* out, PyArena* arena)
{
    PyObject* tmp = NULL;
    expr_ty context_expr;
    expr_ty optional_vars;
    if (_PyObject_HasAttrId(obj, &PyId_context_expr)) {
        int res;
        tmp = _PyObject_GetAttrId(obj, &PyId_context_expr);
        if (tmp == NULL) goto failed;
        res = obj2ast_expr(tmp, &context_expr, arena);
        if (res != 0) goto failed;
        Py_CLEAR(tmp);
    } else {
        PyErr_SetString(PyExc_TypeError, "required field \"context_expr\" missing from withitem");
        return 1;
    }
    if (exists_not_none(obj, &PyId_optional_vars)) {
        int res;
        tmp = _PyObject_GetAttrId(obj, &PyId_optional_vars);
        if (tmp == NULL) goto failed;
        res = obj2ast_expr(tmp, &optional_vars, arena);
        if (res != 0) goto failed;
        Py_CLEAR(tmp);
    } else {
        optional_vars = NULL;
    }
    *out = withitem(context_expr, optional_vars, arena);
    return 0;
failed:
    Py_XDECREF(tmp);
    return 1;
}