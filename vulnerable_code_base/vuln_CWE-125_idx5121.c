ast2obj_keyword(void* _o)
{
    keyword_ty o = (keyword_ty)_o;
    PyObject *result = NULL, *value = NULL;
    if (!o) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    result = PyType_GenericNew(keyword_type, NULL, NULL);
    if (!result) return NULL;
    value = ast2obj_identifier(o->arg);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_arg, value) == -1)
        goto failed;
    Py_DECREF(value);
    value = ast2obj_expr(o->value);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_value, value) == -1)
        goto failed;
    Py_DECREF(value);
    return result;
failed:
    Py_XDECREF(value);
    Py_XDECREF(result);
    return NULL;
}