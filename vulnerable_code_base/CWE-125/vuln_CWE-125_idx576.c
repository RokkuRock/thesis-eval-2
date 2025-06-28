obj2ast_arguments(PyObject* obj, arguments_ty* out, PyArena* arena)
{
    PyObject* tmp = NULL;
    asdl_seq* args;
    arg_ty vararg;
    asdl_seq* kwonlyargs;
    asdl_seq* kw_defaults;
    arg_ty kwarg;
    asdl_seq* defaults;
    if (_PyObject_HasAttrId(obj, &PyId_args)) {
        int res;
        Py_ssize_t len;
        Py_ssize_t i;
        tmp = _PyObject_GetAttrId(obj, &PyId_args);
        if (tmp == NULL) goto failed;
        if (!PyList_Check(tmp)) {
            PyErr_Format(PyExc_TypeError, "arguments field \"args\" must be a list, not a %.200s", tmp->ob_type->tp_name);
            goto failed;
        }
        len = PyList_GET_SIZE(tmp);
        args = _Ta3_asdl_seq_new(len, arena);
        if (args == NULL) goto failed;
        for (i = 0; i < len; i++) {
            arg_ty value;
            res = obj2ast_arg(PyList_GET_ITEM(tmp, i), &value, arena);
            if (res != 0) goto failed;
            if (len != PyList_GET_SIZE(tmp)) {
                PyErr_SetString(PyExc_RuntimeError, "arguments field \"args\" changed size during iteration");
                goto failed;
            }
            asdl_seq_SET(args, i, value);
        }
        Py_CLEAR(tmp);
    } else {
        PyErr_SetString(PyExc_TypeError, "required field \"args\" missing from arguments");
        return 1;
    }
    if (exists_not_none(obj, &PyId_vararg)) {
        int res;
        tmp = _PyObject_GetAttrId(obj, &PyId_vararg);
        if (tmp == NULL) goto failed;
        res = obj2ast_arg(tmp, &vararg, arena);
        if (res != 0) goto failed;
        Py_CLEAR(tmp);
    } else {
        vararg = NULL;
    }
    if (_PyObject_HasAttrId(obj, &PyId_kwonlyargs)) {
        int res;
        Py_ssize_t len;
        Py_ssize_t i;
        tmp = _PyObject_GetAttrId(obj, &PyId_kwonlyargs);
        if (tmp == NULL) goto failed;
        if (!PyList_Check(tmp)) {
            PyErr_Format(PyExc_TypeError, "arguments field \"kwonlyargs\" must be a list, not a %.200s", tmp->ob_type->tp_name);
            goto failed;
        }
        len = PyList_GET_SIZE(tmp);
        kwonlyargs = _Ta3_asdl_seq_new(len, arena);
        if (kwonlyargs == NULL) goto failed;
        for (i = 0; i < len; i++) {
            arg_ty value;
            res = obj2ast_arg(PyList_GET_ITEM(tmp, i), &value, arena);
            if (res != 0) goto failed;
            if (len != PyList_GET_SIZE(tmp)) {
                PyErr_SetString(PyExc_RuntimeError, "arguments field \"kwonlyargs\" changed size during iteration");
                goto failed;
            }
            asdl_seq_SET(kwonlyargs, i, value);
        }
        Py_CLEAR(tmp);
    } else {
        PyErr_SetString(PyExc_TypeError, "required field \"kwonlyargs\" missing from arguments");
        return 1;
    }
    if (_PyObject_HasAttrId(obj, &PyId_kw_defaults)) {
        int res;
        Py_ssize_t len;
        Py_ssize_t i;
        tmp = _PyObject_GetAttrId(obj, &PyId_kw_defaults);
        if (tmp == NULL) goto failed;
        if (!PyList_Check(tmp)) {
            PyErr_Format(PyExc_TypeError, "arguments field \"kw_defaults\" must be a list, not a %.200s", tmp->ob_type->tp_name);
            goto failed;
        }
        len = PyList_GET_SIZE(tmp);
        kw_defaults = _Ta3_asdl_seq_new(len, arena);
        if (kw_defaults == NULL) goto failed;
        for (i = 0; i < len; i++) {
            expr_ty value;
            res = obj2ast_expr(PyList_GET_ITEM(tmp, i), &value, arena);
            if (res != 0) goto failed;
            if (len != PyList_GET_SIZE(tmp)) {
                PyErr_SetString(PyExc_RuntimeError, "arguments field \"kw_defaults\" changed size during iteration");
                goto failed;
            }
            asdl_seq_SET(kw_defaults, i, value);
        }
        Py_CLEAR(tmp);
    } else {
        PyErr_SetString(PyExc_TypeError, "required field \"kw_defaults\" missing from arguments");
        return 1;
    }
    if (exists_not_none(obj, &PyId_kwarg)) {
        int res;
        tmp = _PyObject_GetAttrId(obj, &PyId_kwarg);
        if (tmp == NULL) goto failed;
        res = obj2ast_arg(tmp, &kwarg, arena);
        if (res != 0) goto failed;
        Py_CLEAR(tmp);
    } else {
        kwarg = NULL;
    }
    if (_PyObject_HasAttrId(obj, &PyId_defaults)) {
        int res;
        Py_ssize_t len;
        Py_ssize_t i;
        tmp = _PyObject_GetAttrId(obj, &PyId_defaults);
        if (tmp == NULL) goto failed;
        if (!PyList_Check(tmp)) {
            PyErr_Format(PyExc_TypeError, "arguments field \"defaults\" must be a list, not a %.200s", tmp->ob_type->tp_name);
            goto failed;
        }
        len = PyList_GET_SIZE(tmp);
        defaults = _Ta3_asdl_seq_new(len, arena);
        if (defaults == NULL) goto failed;
        for (i = 0; i < len; i++) {
            expr_ty value;
            res = obj2ast_expr(PyList_GET_ITEM(tmp, i), &value, arena);
            if (res != 0) goto failed;
            if (len != PyList_GET_SIZE(tmp)) {
                PyErr_SetString(PyExc_RuntimeError, "arguments field \"defaults\" changed size during iteration");
                goto failed;
            }
            asdl_seq_SET(defaults, i, value);
        }
        Py_CLEAR(tmp);
    } else {
        PyErr_SetString(PyExc_TypeError, "required field \"defaults\" missing from arguments");
        return 1;
    }
    *out = arguments(args, vararg, kwonlyargs, kw_defaults, kwarg, defaults,
                     arena);
    return 0;
failed:
    Py_XDECREF(tmp);
    return 1;
}