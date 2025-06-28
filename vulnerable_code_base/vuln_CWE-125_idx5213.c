ast2obj_arguments(void* _o)
{
    arguments_ty o = (arguments_ty)_o;
    PyObject *result = NULL, *value = NULL;
    if (!o) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    result = PyType_GenericNew(arguments_type, NULL, NULL);
    if (!result) return NULL;
    value = ast2obj_list(o->args, ast2obj_arg);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_args, value) == -1)
        goto failed;
    Py_DECREF(value);
    value = ast2obj_arg(o->vararg);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_vararg, value) == -1)
        goto failed;
    Py_DECREF(value);
    value = ast2obj_list(o->kwonlyargs, ast2obj_arg);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_kwonlyargs, value) == -1)
        goto failed;
    Py_DECREF(value);
    value = ast2obj_list(o->kw_defaults, ast2obj_expr);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_kw_defaults, value) == -1)
        goto failed;
    Py_DECREF(value);
    value = ast2obj_arg(o->kwarg);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_kwarg, value) == -1)
        goto failed;
    Py_DECREF(value);
    value = ast2obj_list(o->defaults, ast2obj_expr);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_defaults, value) == -1)
        goto failed;
    Py_DECREF(value);
    return result;
failed:
    Py_XDECREF(value);
    Py_XDECREF(result);
    return NULL;
}