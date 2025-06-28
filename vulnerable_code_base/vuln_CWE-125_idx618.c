obj2ast_slice(PyObject* obj, slice_ty* out, PyArena* arena)
{
    int isinstance;
    PyObject *tmp = NULL;
    if (obj == Py_None) {
        *out = NULL;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Slice_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        expr_ty lower;
        expr_ty upper;
        expr_ty step;
        if (exists_not_none(obj, &PyId_lower)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_lower);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &lower, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            lower = NULL;
        }
        if (exists_not_none(obj, &PyId_upper)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_upper);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &upper, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            upper = NULL;
        }
        if (exists_not_none(obj, &PyId_step)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_step);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &step, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            step = NULL;
        }
        *out = Slice(lower, upper, step, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)ExtSlice_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        asdl_seq* dims;
        if (_PyObject_HasAttrId(obj, &PyId_dims)) {
            int res;
            Py_ssize_t len;
            Py_ssize_t i;
            tmp = _PyObject_GetAttrId(obj, &PyId_dims);
            if (tmp == NULL) goto failed;
            if (!PyList_Check(tmp)) {
                PyErr_Format(PyExc_TypeError, "ExtSlice field \"dims\" must be a list, not a %.200s", tmp->ob_type->tp_name);
                goto failed;
            }
            len = PyList_GET_SIZE(tmp);
            dims = _Ta3_asdl_seq_new(len, arena);
            if (dims == NULL) goto failed;
            for (i = 0; i < len; i++) {
                slice_ty value;
                res = obj2ast_slice(PyList_GET_ITEM(tmp, i), &value, arena);
                if (res != 0) goto failed;
                if (len != PyList_GET_SIZE(tmp)) {
                    PyErr_SetString(PyExc_RuntimeError, "ExtSlice field \"dims\" changed size during iteration");
                    goto failed;
                }
                asdl_seq_SET(dims, i, value);
            }
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"dims\" missing from ExtSlice");
            return 1;
        }
        *out = ExtSlice(dims, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Index_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        expr_ty value;
        if (_PyObject_HasAttrId(obj, &PyId_value)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_value);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &value, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"value\" missing from Index");
            return 1;
        }
        *out = Index(value, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    PyErr_Format(PyExc_TypeError, "expected some sort of slice, but got %R", obj);
    failed:
    Py_XDECREF(tmp);
    return 1;
}