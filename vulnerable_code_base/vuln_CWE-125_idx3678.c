ast2obj_type_ignore(void* _o)
{
    type_ignore_ty o = (type_ignore_ty)_o;
    PyObject *result = NULL, *value = NULL;
    if (!o) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    switch (o->kind) {
    case TypeIgnore_kind:
        result = PyType_GenericNew(TypeIgnore_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_int(o->v.TypeIgnore.lineno);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_lineno, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    }
    return result;
failed:
    Py_XDECREF(value);
    Py_XDECREF(result);
    return NULL;
}