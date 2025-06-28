ast2obj_alias(void* _o)
{
    alias_ty o = (alias_ty)_o;
    PyObject *result = NULL, *value = NULL;
    if (!o) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    result = PyType_GenericNew(alias_type, NULL, NULL);
    if (!result) return NULL;
    value = ast2obj_identifier(o->name);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_name, value) == -1)
        goto failed;
    Py_DECREF(value);
    value = ast2obj_identifier(o->asname);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_asname, value) == -1)
        goto failed;
    Py_DECREF(value);
    return result;
failed:
    Py_XDECREF(value);
    Py_XDECREF(result);
    return NULL;
}