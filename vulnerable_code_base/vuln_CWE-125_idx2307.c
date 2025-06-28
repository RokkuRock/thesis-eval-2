obj2ast_arg(PyObject* obj, arg_ty* out, PyArena* arena)
{
    PyObject* tmp = NULL;
    identifier arg;
    expr_ty annotation;
    string type_comment;
    int lineno;
    int col_offset;
    if (_PyObject_HasAttrId(obj, &PyId_arg)) {
        int res;
        tmp = _PyObject_GetAttrId(obj, &PyId_arg);
        if (tmp == NULL) goto failed;
        res = obj2ast_identifier(tmp, &arg, arena);
        if (res != 0) goto failed;
        Py_CLEAR(tmp);
    } else {
        PyErr_SetString(PyExc_TypeError, "required field \"arg\" missing from arg");
        return 1;
    }
    if (exists_not_none(obj, &PyId_annotation)) {
        int res;
        tmp = _PyObject_GetAttrId(obj, &PyId_annotation);
        if (tmp == NULL) goto failed;
        res = obj2ast_expr(tmp, &annotation, arena);
        if (res != 0) goto failed;
        Py_CLEAR(tmp);
    } else {
        annotation = NULL;
    }
    if (exists_not_none(obj, &PyId_type_comment)) {
        int res;
        tmp = _PyObject_GetAttrId(obj, &PyId_type_comment);
        if (tmp == NULL) goto failed;
        res = obj2ast_string(tmp, &type_comment, arena);
        if (res != 0) goto failed;
        Py_CLEAR(tmp);
    } else {
        type_comment = NULL;
    }
    if (_PyObject_HasAttrId(obj, &PyId_lineno)) {
        int res;
        tmp = _PyObject_GetAttrId(obj, &PyId_lineno);
        if (tmp == NULL) goto failed;
        res = obj2ast_int(tmp, &lineno, arena);
        if (res != 0) goto failed;
        Py_CLEAR(tmp);
    } else {
        PyErr_SetString(PyExc_TypeError, "required field \"lineno\" missing from arg");
        return 1;
    }
    if (_PyObject_HasAttrId(obj, &PyId_col_offset)) {
        int res;
        tmp = _PyObject_GetAttrId(obj, &PyId_col_offset);
        if (tmp == NULL) goto failed;
        res = obj2ast_int(tmp, &col_offset, arena);
        if (res != 0) goto failed;
        Py_CLEAR(tmp);
    } else {
        PyErr_SetString(PyExc_TypeError, "required field \"col_offset\" missing from arg");
        return 1;
    }
    *out = arg(arg, annotation, type_comment, lineno, col_offset, arena);
    return 0;
failed:
    Py_XDECREF(tmp);
    return 1;
}