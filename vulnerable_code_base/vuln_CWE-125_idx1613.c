ast2obj_withitem(void* _o)
{
    withitem_ty o = (withitem_ty)_o;
    PyObject *result = NULL, *value = NULL;
    if (!o) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    result = PyType_GenericNew(withitem_type, NULL, NULL);
    if (!result) return NULL;
    value = ast2obj_expr(o->context_expr);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_context_expr, value) == -1)
        goto failed;
    Py_DECREF(value);
    value = ast2obj_expr(o->optional_vars);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_optional_vars, value) == -1)
        goto failed;
    Py_DECREF(value);
    return result;
failed:
    Py_XDECREF(value);
    Py_XDECREF(result);
    return NULL;
}