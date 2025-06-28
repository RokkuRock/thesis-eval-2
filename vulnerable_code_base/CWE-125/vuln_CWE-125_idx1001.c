obj2ast_excepthandler(PyObject* obj, excepthandler_ty* out, PyArena* arena)
{
    int isinstance;
    PyObject *tmp = NULL;
    int lineno;
    int col_offset;
    if (obj == Py_None) {
        *out = NULL;
        return 0;
    }
    if (_PyObject_HasAttrId(obj, &PyId_lineno)) {
        int res;
        tmp = _PyObject_GetAttrId(obj, &PyId_lineno);
        if (tmp == NULL) goto failed;
        res = obj2ast_int(tmp, &lineno, arena);
        if (res != 0) goto failed;
        Py_CLEAR(tmp);
    } else {
        PyErr_SetString(PyExc_TypeError, "required field \"lineno\" missing from excepthandler");
        return 1;
    }
    if (_PyObject_HasAttrId(obj, &PyId_col_offset)) {
        int res;
        tmp = _PyObject_GetAttrId(obj, &PyId_col_offset);
        if (tmp == NULL) goto failed;
        res = obj2ast_int(tmp, &col_offset, arena);
        if (res != 0) goto failed;
        Py_CLEAR(tmp);
    } else {
        PyErr_SetString(PyExc_TypeError, "required field \"col_offset\" missing from excepthandler");
        return 1;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)ExceptHandler_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        expr_ty type;
        identifier name;
        asdl_seq* body;
        if (exists_not_none(obj, &PyId_type)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_type);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &type, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            type = NULL;
        }
        if (exists_not_none(obj, &PyId_name)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_name);
            if (tmp == NULL) goto failed;
            res = obj2ast_identifier(tmp, &name, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            name = NULL;
        }
        if (_PyObject_HasAttrId(obj, &PyId_body)) {
            int res;
            Py_ssize_t len;
            Py_ssize_t i;
            tmp = _PyObject_GetAttrId(obj, &PyId_body);
            if (tmp == NULL) goto failed;
            if (!PyList_Check(tmp)) {
                PyErr_Format(PyExc_TypeError, "ExceptHandler field \"body\" must be a list, not a %.200s", tmp->ob_type->tp_name);
                goto failed;
            }
            len = PyList_GET_SIZE(tmp);
            body = _Ta3_asdl_seq_new(len, arena);
            if (body == NULL) goto failed;
            for (i = 0; i < len; i++) {
                stmt_ty value;
                res = obj2ast_stmt(PyList_GET_ITEM(tmp, i), &value, arena);
                if (res != 0) goto failed;
                if (len != PyList_GET_SIZE(tmp)) {
                    PyErr_SetString(PyExc_RuntimeError, "ExceptHandler field \"body\" changed size during iteration");
                    goto failed;
                }
                asdl_seq_SET(body, i, value);
            }
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"body\" missing from ExceptHandler");
            return 1;
        }
        *out = ExceptHandler(type, name, body, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    PyErr_Format(PyExc_TypeError, "expected some sort of excepthandler, but got %R", obj);
    failed:
    Py_XDECREF(tmp);
    return 1;
}