ast2obj_excepthandler(void* _o)
{
    excepthandler_ty o = (excepthandler_ty)_o;
    PyObject *result = NULL, *value = NULL;
    if (!o) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    switch (o->kind) {
    case ExceptHandler_kind:
        result = PyType_GenericNew(ExceptHandler_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_expr(o->v.ExceptHandler.type);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_type, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_identifier(o->v.ExceptHandler.name);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_name, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_list(o->v.ExceptHandler.body, ast2obj_stmt);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_body, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    }
    value = ast2obj_int(o->lineno);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_lineno, value) < 0)
        goto failed;
    Py_DECREF(value);
    value = ast2obj_int(o->col_offset);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_col_offset, value) < 0)
        goto failed;
    Py_DECREF(value);
    return result;
failed:
    Py_XDECREF(value);
    Py_XDECREF(result);
    return NULL;
}