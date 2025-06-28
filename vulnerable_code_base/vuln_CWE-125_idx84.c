ast2obj_slice(void* _o)
{
    slice_ty o = (slice_ty)_o;
    PyObject *result = NULL, *value = NULL;
    if (!o) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    switch (o->kind) {
    case Slice_kind:
        result = PyType_GenericNew(Slice_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_expr(o->v.Slice.lower);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_lower, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_expr(o->v.Slice.upper);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_upper, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_expr(o->v.Slice.step);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_step, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case ExtSlice_kind:
        result = PyType_GenericNew(ExtSlice_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_list(o->v.ExtSlice.dims, ast2obj_slice);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_dims, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case Index_kind:
        result = PyType_GenericNew(Index_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_expr(o->v.Index.value);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_value, value) == -1)
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