obj2ast_expr(PyObject* obj, expr_ty* out, PyArena* arena)
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
        PyErr_SetString(PyExc_TypeError, "required field \"lineno\" missing from expr");
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
        PyErr_SetString(PyExc_TypeError, "required field \"col_offset\" missing from expr");
        return 1;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)BoolOp_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        boolop_ty op;
        asdl_seq* values;
        if (_PyObject_HasAttrId(obj, &PyId_op)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_op);
            if (tmp == NULL) goto failed;
            res = obj2ast_boolop(tmp, &op, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"op\" missing from BoolOp");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_values)) {
            int res;
            Py_ssize_t len;
            Py_ssize_t i;
            tmp = _PyObject_GetAttrId(obj, &PyId_values);
            if (tmp == NULL) goto failed;
            if (!PyList_Check(tmp)) {
                PyErr_Format(PyExc_TypeError, "BoolOp field \"values\" must be a list, not a %.200s", tmp->ob_type->tp_name);
                goto failed;
            }
            len = PyList_GET_SIZE(tmp);
            values = _Ta3_asdl_seq_new(len, arena);
            if (values == NULL) goto failed;
            for (i = 0; i < len; i++) {
                expr_ty value;
                res = obj2ast_expr(PyList_GET_ITEM(tmp, i), &value, arena);
                if (res != 0) goto failed;
                if (len != PyList_GET_SIZE(tmp)) {
                    PyErr_SetString(PyExc_RuntimeError, "BoolOp field \"values\" changed size during iteration");
                    goto failed;
                }
                asdl_seq_SET(values, i, value);
            }
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"values\" missing from BoolOp");
            return 1;
        }
        *out = BoolOp(op, values, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)BinOp_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        expr_ty left;
        operator_ty op;
        expr_ty right;
        if (_PyObject_HasAttrId(obj, &PyId_left)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_left);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &left, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"left\" missing from BinOp");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_op)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_op);
            if (tmp == NULL) goto failed;
            res = obj2ast_operator(tmp, &op, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"op\" missing from BinOp");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_right)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_right);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &right, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"right\" missing from BinOp");
            return 1;
        }
        *out = BinOp(left, op, right, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)UnaryOp_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        unaryop_ty op;
        expr_ty operand;
        if (_PyObject_HasAttrId(obj, &PyId_op)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_op);
            if (tmp == NULL) goto failed;
            res = obj2ast_unaryop(tmp, &op, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"op\" missing from UnaryOp");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_operand)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_operand);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &operand, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"operand\" missing from UnaryOp");
            return 1;
        }
        *out = UnaryOp(op, operand, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Lambda_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        arguments_ty args;
        expr_ty body;
        if (_PyObject_HasAttrId(obj, &PyId_args)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_args);
            if (tmp == NULL) goto failed;
            res = obj2ast_arguments(tmp, &args, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"args\" missing from Lambda");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_body)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_body);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &body, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"body\" missing from Lambda");
            return 1;
        }
        *out = Lambda(args, body, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)IfExp_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        expr_ty test;
        expr_ty body;
        expr_ty orelse;
        if (_PyObject_HasAttrId(obj, &PyId_test)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_test);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &test, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"test\" missing from IfExp");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_body)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_body);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &body, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"body\" missing from IfExp");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_orelse)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_orelse);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &orelse, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"orelse\" missing from IfExp");
            return 1;
        }
        *out = IfExp(test, body, orelse, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Dict_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        asdl_seq* keys;
        asdl_seq* values;
        if (_PyObject_HasAttrId(obj, &PyId_keys)) {
            int res;
            Py_ssize_t len;
            Py_ssize_t i;
            tmp = _PyObject_GetAttrId(obj, &PyId_keys);
            if (tmp == NULL) goto failed;
            if (!PyList_Check(tmp)) {
                PyErr_Format(PyExc_TypeError, "Dict field \"keys\" must be a list, not a %.200s", tmp->ob_type->tp_name);
                goto failed;
            }
            len = PyList_GET_SIZE(tmp);
            keys = _Ta3_asdl_seq_new(len, arena);
            if (keys == NULL) goto failed;
            for (i = 0; i < len; i++) {
                expr_ty value;
                res = obj2ast_expr(PyList_GET_ITEM(tmp, i), &value, arena);
                if (res != 0) goto failed;
                if (len != PyList_GET_SIZE(tmp)) {
                    PyErr_SetString(PyExc_RuntimeError, "Dict field \"keys\" changed size during iteration");
                    goto failed;
                }
                asdl_seq_SET(keys, i, value);
            }
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"keys\" missing from Dict");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_values)) {
            int res;
            Py_ssize_t len;
            Py_ssize_t i;
            tmp = _PyObject_GetAttrId(obj, &PyId_values);
            if (tmp == NULL) goto failed;
            if (!PyList_Check(tmp)) {
                PyErr_Format(PyExc_TypeError, "Dict field \"values\" must be a list, not a %.200s", tmp->ob_type->tp_name);
                goto failed;
            }
            len = PyList_GET_SIZE(tmp);
            values = _Ta3_asdl_seq_new(len, arena);
            if (values == NULL) goto failed;
            for (i = 0; i < len; i++) {
                expr_ty value;
                res = obj2ast_expr(PyList_GET_ITEM(tmp, i), &value, arena);
                if (res != 0) goto failed;
                if (len != PyList_GET_SIZE(tmp)) {
                    PyErr_SetString(PyExc_RuntimeError, "Dict field \"values\" changed size during iteration");
                    goto failed;
                }
                asdl_seq_SET(values, i, value);
            }
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"values\" missing from Dict");
            return 1;
        }
        *out = Dict(keys, values, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Set_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        asdl_seq* elts;
        if (_PyObject_HasAttrId(obj, &PyId_elts)) {
            int res;
            Py_ssize_t len;
            Py_ssize_t i;
            tmp = _PyObject_GetAttrId(obj, &PyId_elts);
            if (tmp == NULL) goto failed;
            if (!PyList_Check(tmp)) {
                PyErr_Format(PyExc_TypeError, "Set field \"elts\" must be a list, not a %.200s", tmp->ob_type->tp_name);
                goto failed;
            }
            len = PyList_GET_SIZE(tmp);
            elts = _Ta3_asdl_seq_new(len, arena);
            if (elts == NULL) goto failed;
            for (i = 0; i < len; i++) {
                expr_ty value;
                res = obj2ast_expr(PyList_GET_ITEM(tmp, i), &value, arena);
                if (res != 0) goto failed;
                if (len != PyList_GET_SIZE(tmp)) {
                    PyErr_SetString(PyExc_RuntimeError, "Set field \"elts\" changed size during iteration");
                    goto failed;
                }
                asdl_seq_SET(elts, i, value);
            }
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"elts\" missing from Set");
            return 1;
        }
        *out = Set(elts, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)ListComp_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        expr_ty elt;
        asdl_seq* generators;
        if (_PyObject_HasAttrId(obj, &PyId_elt)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_elt);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &elt, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"elt\" missing from ListComp");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_generators)) {
            int res;
            Py_ssize_t len;
            Py_ssize_t i;
            tmp = _PyObject_GetAttrId(obj, &PyId_generators);
            if (tmp == NULL) goto failed;
            if (!PyList_Check(tmp)) {
                PyErr_Format(PyExc_TypeError, "ListComp field \"generators\" must be a list, not a %.200s", tmp->ob_type->tp_name);
                goto failed;
            }
            len = PyList_GET_SIZE(tmp);
            generators = _Ta3_asdl_seq_new(len, arena);
            if (generators == NULL) goto failed;
            for (i = 0; i < len; i++) {
                comprehension_ty value;
                res = obj2ast_comprehension(PyList_GET_ITEM(tmp, i), &value, arena);
                if (res != 0) goto failed;
                if (len != PyList_GET_SIZE(tmp)) {
                    PyErr_SetString(PyExc_RuntimeError, "ListComp field \"generators\" changed size during iteration");
                    goto failed;
                }
                asdl_seq_SET(generators, i, value);
            }
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"generators\" missing from ListComp");
            return 1;
        }
        *out = ListComp(elt, generators, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)SetComp_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        expr_ty elt;
        asdl_seq* generators;
        if (_PyObject_HasAttrId(obj, &PyId_elt)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_elt);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &elt, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"elt\" missing from SetComp");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_generators)) {
            int res;
            Py_ssize_t len;
            Py_ssize_t i;
            tmp = _PyObject_GetAttrId(obj, &PyId_generators);
            if (tmp == NULL) goto failed;
            if (!PyList_Check(tmp)) {
                PyErr_Format(PyExc_TypeError, "SetComp field \"generators\" must be a list, not a %.200s", tmp->ob_type->tp_name);
                goto failed;
            }
            len = PyList_GET_SIZE(tmp);
            generators = _Ta3_asdl_seq_new(len, arena);
            if (generators == NULL) goto failed;
            for (i = 0; i < len; i++) {
                comprehension_ty value;
                res = obj2ast_comprehension(PyList_GET_ITEM(tmp, i), &value, arena);
                if (res != 0) goto failed;
                if (len != PyList_GET_SIZE(tmp)) {
                    PyErr_SetString(PyExc_RuntimeError, "SetComp field \"generators\" changed size during iteration");
                    goto failed;
                }
                asdl_seq_SET(generators, i, value);
            }
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"generators\" missing from SetComp");
            return 1;
        }
        *out = SetComp(elt, generators, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)DictComp_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        expr_ty key;
        expr_ty value;
        asdl_seq* generators;
        if (_PyObject_HasAttrId(obj, &PyId_key)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_key);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &key, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"key\" missing from DictComp");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_value)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_value);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &value, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"value\" missing from DictComp");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_generators)) {
            int res;
            Py_ssize_t len;
            Py_ssize_t i;
            tmp = _PyObject_GetAttrId(obj, &PyId_generators);
            if (tmp == NULL) goto failed;
            if (!PyList_Check(tmp)) {
                PyErr_Format(PyExc_TypeError, "DictComp field \"generators\" must be a list, not a %.200s", tmp->ob_type->tp_name);
                goto failed;
            }
            len = PyList_GET_SIZE(tmp);
            generators = _Ta3_asdl_seq_new(len, arena);
            if (generators == NULL) goto failed;
            for (i = 0; i < len; i++) {
                comprehension_ty value;
                res = obj2ast_comprehension(PyList_GET_ITEM(tmp, i), &value, arena);
                if (res != 0) goto failed;
                if (len != PyList_GET_SIZE(tmp)) {
                    PyErr_SetString(PyExc_RuntimeError, "DictComp field \"generators\" changed size during iteration");
                    goto failed;
                }
                asdl_seq_SET(generators, i, value);
            }
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"generators\" missing from DictComp");
            return 1;
        }
        *out = DictComp(key, value, generators, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)GeneratorExp_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        expr_ty elt;
        asdl_seq* generators;
        if (_PyObject_HasAttrId(obj, &PyId_elt)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_elt);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &elt, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"elt\" missing from GeneratorExp");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_generators)) {
            int res;
            Py_ssize_t len;
            Py_ssize_t i;
            tmp = _PyObject_GetAttrId(obj, &PyId_generators);
            if (tmp == NULL) goto failed;
            if (!PyList_Check(tmp)) {
                PyErr_Format(PyExc_TypeError, "GeneratorExp field \"generators\" must be a list, not a %.200s", tmp->ob_type->tp_name);
                goto failed;
            }
            len = PyList_GET_SIZE(tmp);
            generators = _Ta3_asdl_seq_new(len, arena);
            if (generators == NULL) goto failed;
            for (i = 0; i < len; i++) {
                comprehension_ty value;
                res = obj2ast_comprehension(PyList_GET_ITEM(tmp, i), &value, arena);
                if (res != 0) goto failed;
                if (len != PyList_GET_SIZE(tmp)) {
                    PyErr_SetString(PyExc_RuntimeError, "GeneratorExp field \"generators\" changed size during iteration");
                    goto failed;
                }
                asdl_seq_SET(generators, i, value);
            }
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"generators\" missing from GeneratorExp");
            return 1;
        }
        *out = GeneratorExp(elt, generators, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Await_type);
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
            PyErr_SetString(PyExc_TypeError, "required field \"value\" missing from Await");
            return 1;
        }
        *out = Await(value, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Yield_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        expr_ty value;
        if (exists_not_none(obj, &PyId_value)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_value);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &value, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            value = NULL;
        }
        *out = Yield(value, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)YieldFrom_type);
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
            PyErr_SetString(PyExc_TypeError, "required field \"value\" missing from YieldFrom");
            return 1;
        }
        *out = YieldFrom(value, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Compare_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        expr_ty left;
        asdl_int_seq* ops;
        asdl_seq* comparators;
        if (_PyObject_HasAttrId(obj, &PyId_left)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_left);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &left, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"left\" missing from Compare");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_ops)) {
            int res;
            Py_ssize_t len;
            Py_ssize_t i;
            tmp = _PyObject_GetAttrId(obj, &PyId_ops);
            if (tmp == NULL) goto failed;
            if (!PyList_Check(tmp)) {
                PyErr_Format(PyExc_TypeError, "Compare field \"ops\" must be a list, not a %.200s", tmp->ob_type->tp_name);
                goto failed;
            }
            len = PyList_GET_SIZE(tmp);
            ops = _Ta3_asdl_int_seq_new(len, arena);
            if (ops == NULL) goto failed;
            for (i = 0; i < len; i++) {
                cmpop_ty value;
                res = obj2ast_cmpop(PyList_GET_ITEM(tmp, i), &value, arena);
                if (res != 0) goto failed;
                if (len != PyList_GET_SIZE(tmp)) {
                    PyErr_SetString(PyExc_RuntimeError, "Compare field \"ops\" changed size during iteration");
                    goto failed;
                }
                asdl_seq_SET(ops, i, value);
            }
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"ops\" missing from Compare");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_comparators)) {
            int res;
            Py_ssize_t len;
            Py_ssize_t i;
            tmp = _PyObject_GetAttrId(obj, &PyId_comparators);
            if (tmp == NULL) goto failed;
            if (!PyList_Check(tmp)) {
                PyErr_Format(PyExc_TypeError, "Compare field \"comparators\" must be a list, not a %.200s", tmp->ob_type->tp_name);
                goto failed;
            }
            len = PyList_GET_SIZE(tmp);
            comparators = _Ta3_asdl_seq_new(len, arena);
            if (comparators == NULL) goto failed;
            for (i = 0; i < len; i++) {
                expr_ty value;
                res = obj2ast_expr(PyList_GET_ITEM(tmp, i), &value, arena);
                if (res != 0) goto failed;
                if (len != PyList_GET_SIZE(tmp)) {
                    PyErr_SetString(PyExc_RuntimeError, "Compare field \"comparators\" changed size during iteration");
                    goto failed;
                }
                asdl_seq_SET(comparators, i, value);
            }
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"comparators\" missing from Compare");
            return 1;
        }
        *out = Compare(left, ops, comparators, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Call_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        expr_ty func;
        asdl_seq* args;
        asdl_seq* keywords;
        if (_PyObject_HasAttrId(obj, &PyId_func)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_func);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &func, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"func\" missing from Call");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_args)) {
            int res;
            Py_ssize_t len;
            Py_ssize_t i;
            tmp = _PyObject_GetAttrId(obj, &PyId_args);
            if (tmp == NULL) goto failed;
            if (!PyList_Check(tmp)) {
                PyErr_Format(PyExc_TypeError, "Call field \"args\" must be a list, not a %.200s", tmp->ob_type->tp_name);
                goto failed;
            }
            len = PyList_GET_SIZE(tmp);
            args = _Ta3_asdl_seq_new(len, arena);
            if (args == NULL) goto failed;
            for (i = 0; i < len; i++) {
                expr_ty value;
                res = obj2ast_expr(PyList_GET_ITEM(tmp, i), &value, arena);
                if (res != 0) goto failed;
                if (len != PyList_GET_SIZE(tmp)) {
                    PyErr_SetString(PyExc_RuntimeError, "Call field \"args\" changed size during iteration");
                    goto failed;
                }
                asdl_seq_SET(args, i, value);
            }
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"args\" missing from Call");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_keywords)) {
            int res;
            Py_ssize_t len;
            Py_ssize_t i;
            tmp = _PyObject_GetAttrId(obj, &PyId_keywords);
            if (tmp == NULL) goto failed;
            if (!PyList_Check(tmp)) {
                PyErr_Format(PyExc_TypeError, "Call field \"keywords\" must be a list, not a %.200s", tmp->ob_type->tp_name);
                goto failed;
            }
            len = PyList_GET_SIZE(tmp);
            keywords = _Ta3_asdl_seq_new(len, arena);
            if (keywords == NULL) goto failed;
            for (i = 0; i < len; i++) {
                keyword_ty value;
                res = obj2ast_keyword(PyList_GET_ITEM(tmp, i), &value, arena);
                if (res != 0) goto failed;
                if (len != PyList_GET_SIZE(tmp)) {
                    PyErr_SetString(PyExc_RuntimeError, "Call field \"keywords\" changed size during iteration");
                    goto failed;
                }
                asdl_seq_SET(keywords, i, value);
            }
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"keywords\" missing from Call");
            return 1;
        }
        *out = Call(func, args, keywords, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Num_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        object n;
        if (_PyObject_HasAttrId(obj, &PyId_n)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_n);
            if (tmp == NULL) goto failed;
            res = obj2ast_object(tmp, &n, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"n\" missing from Num");
            return 1;
        }
        *out = Num(n, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Str_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        string s;
        string kind;
        if (_PyObject_HasAttrId(obj, &PyId_s)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_s);
            if (tmp == NULL) goto failed;
            res = obj2ast_string(tmp, &s, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"s\" missing from Str");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_kind)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_kind);
            if (tmp == NULL) goto failed;
            res = obj2ast_string(tmp, &kind, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"kind\" missing from Str");
            return 1;
        }
        *out = Str(s, kind, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)FormattedValue_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        expr_ty value;
        int conversion;
        expr_ty format_spec;
        if (_PyObject_HasAttrId(obj, &PyId_value)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_value);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &value, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"value\" missing from FormattedValue");
            return 1;
        }
        if (exists_not_none(obj, &PyId_conversion)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_conversion);
            if (tmp == NULL) goto failed;
            res = obj2ast_int(tmp, &conversion, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            conversion = 0;
        }
        if (exists_not_none(obj, &PyId_format_spec)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_format_spec);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &format_spec, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            format_spec = NULL;
        }
        *out = FormattedValue(value, conversion, format_spec, lineno,
                              col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)JoinedStr_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        asdl_seq* values;
        if (_PyObject_HasAttrId(obj, &PyId_values)) {
            int res;
            Py_ssize_t len;
            Py_ssize_t i;
            tmp = _PyObject_GetAttrId(obj, &PyId_values);
            if (tmp == NULL) goto failed;
            if (!PyList_Check(tmp)) {
                PyErr_Format(PyExc_TypeError, "JoinedStr field \"values\" must be a list, not a %.200s", tmp->ob_type->tp_name);
                goto failed;
            }
            len = PyList_GET_SIZE(tmp);
            values = _Ta3_asdl_seq_new(len, arena);
            if (values == NULL) goto failed;
            for (i = 0; i < len; i++) {
                expr_ty value;
                res = obj2ast_expr(PyList_GET_ITEM(tmp, i), &value, arena);
                if (res != 0) goto failed;
                if (len != PyList_GET_SIZE(tmp)) {
                    PyErr_SetString(PyExc_RuntimeError, "JoinedStr field \"values\" changed size during iteration");
                    goto failed;
                }
                asdl_seq_SET(values, i, value);
            }
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"values\" missing from JoinedStr");
            return 1;
        }
        *out = JoinedStr(values, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Bytes_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        bytes s;
        if (_PyObject_HasAttrId(obj, &PyId_s)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_s);
            if (tmp == NULL) goto failed;
            res = obj2ast_bytes(tmp, &s, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"s\" missing from Bytes");
            return 1;
        }
        *out = Bytes(s, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)NameConstant_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        singleton value;
        if (_PyObject_HasAttrId(obj, &PyId_value)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_value);
            if (tmp == NULL) goto failed;
            res = obj2ast_singleton(tmp, &value, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"value\" missing from NameConstant");
            return 1;
        }
        *out = NameConstant(value, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Ellipsis_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        *out = Ellipsis(lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Constant_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        constant value;
        if (_PyObject_HasAttrId(obj, &PyId_value)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_value);
            if (tmp == NULL) goto failed;
            res = obj2ast_constant(tmp, &value, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"value\" missing from Constant");
            return 1;
        }
        *out = Constant(value, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Attribute_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        expr_ty value;
        identifier attr;
        expr_context_ty ctx;
        if (_PyObject_HasAttrId(obj, &PyId_value)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_value);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &value, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"value\" missing from Attribute");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_attr)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_attr);
            if (tmp == NULL) goto failed;
            res = obj2ast_identifier(tmp, &attr, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"attr\" missing from Attribute");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_ctx)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_ctx);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr_context(tmp, &ctx, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"ctx\" missing from Attribute");
            return 1;
        }
        *out = Attribute(value, attr, ctx, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Subscript_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        expr_ty value;
        slice_ty slice;
        expr_context_ty ctx;
        if (_PyObject_HasAttrId(obj, &PyId_value)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_value);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &value, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"value\" missing from Subscript");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_slice)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_slice);
            if (tmp == NULL) goto failed;
            res = obj2ast_slice(tmp, &slice, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"slice\" missing from Subscript");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_ctx)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_ctx);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr_context(tmp, &ctx, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"ctx\" missing from Subscript");
            return 1;
        }
        *out = Subscript(value, slice, ctx, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Starred_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        expr_ty value;
        expr_context_ty ctx;
        if (_PyObject_HasAttrId(obj, &PyId_value)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_value);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr(tmp, &value, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"value\" missing from Starred");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_ctx)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_ctx);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr_context(tmp, &ctx, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"ctx\" missing from Starred");
            return 1;
        }
        *out = Starred(value, ctx, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Name_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        identifier id;
        expr_context_ty ctx;
        if (_PyObject_HasAttrId(obj, &PyId_id)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_id);
            if (tmp == NULL) goto failed;
            res = obj2ast_identifier(tmp, &id, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"id\" missing from Name");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_ctx)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_ctx);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr_context(tmp, &ctx, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"ctx\" missing from Name");
            return 1;
        }
        *out = Name(id, ctx, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)List_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        asdl_seq* elts;
        expr_context_ty ctx;
        if (_PyObject_HasAttrId(obj, &PyId_elts)) {
            int res;
            Py_ssize_t len;
            Py_ssize_t i;
            tmp = _PyObject_GetAttrId(obj, &PyId_elts);
            if (tmp == NULL) goto failed;
            if (!PyList_Check(tmp)) {
                PyErr_Format(PyExc_TypeError, "List field \"elts\" must be a list, not a %.200s", tmp->ob_type->tp_name);
                goto failed;
            }
            len = PyList_GET_SIZE(tmp);
            elts = _Ta3_asdl_seq_new(len, arena);
            if (elts == NULL) goto failed;
            for (i = 0; i < len; i++) {
                expr_ty value;
                res = obj2ast_expr(PyList_GET_ITEM(tmp, i), &value, arena);
                if (res != 0) goto failed;
                if (len != PyList_GET_SIZE(tmp)) {
                    PyErr_SetString(PyExc_RuntimeError, "List field \"elts\" changed size during iteration");
                    goto failed;
                }
                asdl_seq_SET(elts, i, value);
            }
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"elts\" missing from List");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_ctx)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_ctx);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr_context(tmp, &ctx, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"ctx\" missing from List");
            return 1;
        }
        *out = List(elts, ctx, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    isinstance = PyObject_IsInstance(obj, (PyObject*)Tuple_type);
    if (isinstance == -1) {
        return 1;
    }
    if (isinstance) {
        asdl_seq* elts;
        expr_context_ty ctx;
        if (_PyObject_HasAttrId(obj, &PyId_elts)) {
            int res;
            Py_ssize_t len;
            Py_ssize_t i;
            tmp = _PyObject_GetAttrId(obj, &PyId_elts);
            if (tmp == NULL) goto failed;
            if (!PyList_Check(tmp)) {
                PyErr_Format(PyExc_TypeError, "Tuple field \"elts\" must be a list, not a %.200s", tmp->ob_type->tp_name);
                goto failed;
            }
            len = PyList_GET_SIZE(tmp);
            elts = _Ta3_asdl_seq_new(len, arena);
            if (elts == NULL) goto failed;
            for (i = 0; i < len; i++) {
                expr_ty value;
                res = obj2ast_expr(PyList_GET_ITEM(tmp, i), &value, arena);
                if (res != 0) goto failed;
                if (len != PyList_GET_SIZE(tmp)) {
                    PyErr_SetString(PyExc_RuntimeError, "Tuple field \"elts\" changed size during iteration");
                    goto failed;
                }
                asdl_seq_SET(elts, i, value);
            }
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"elts\" missing from Tuple");
            return 1;
        }
        if (_PyObject_HasAttrId(obj, &PyId_ctx)) {
            int res;
            tmp = _PyObject_GetAttrId(obj, &PyId_ctx);
            if (tmp == NULL) goto failed;
            res = obj2ast_expr_context(tmp, &ctx, arena);
            if (res != 0) goto failed;
            Py_CLEAR(tmp);
        } else {
            PyErr_SetString(PyExc_TypeError, "required field \"ctx\" missing from Tuple");
            return 1;
        }
        *out = Tuple(elts, ctx, lineno, col_offset, arena);
        if (*out == NULL) goto failed;
        return 0;
    }
    PyErr_Format(PyExc_TypeError, "expected some sort of expr, but got %R", obj);
    failed:
    Py_XDECREF(tmp);
    return 1;
}