ast2obj_comprehension(void* _o)
{
    comprehension_ty o = (comprehension_ty)_o;
    PyObject *result = NULL, *value = NULL;
    if (!o) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    result = PyType_GenericNew(comprehension_type, NULL, NULL);
    if (!result) return NULL;
    value = ast2obj_expr(o->target);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_target, value) == -1)
        goto failed;
    Py_DECREF(value);
    value = ast2obj_expr(o->iter);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_iter, value) == -1)
        goto failed;
    Py_DECREF(value);
    value = ast2obj_list(o->ifs, ast2obj_expr);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_ifs, value) == -1)
        goto failed;
    Py_DECREF(value);
    value = ast2obj_int(o->is_async);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_is_async, value) == -1)
        goto failed;
    Py_DECREF(value);
    return result;
failed:
    Py_XDECREF(value);
    Py_XDECREF(result);
    return NULL;
}