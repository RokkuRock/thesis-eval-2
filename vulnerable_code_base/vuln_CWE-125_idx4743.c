ast2obj_expr(void* _o)
{
    expr_ty o = (expr_ty)_o;
    PyObject *result = NULL, *value = NULL;
    if (!o) {
        Py_INCREF(Py_None);
        return Py_None;
    }
    switch (o->kind) {
    case BoolOp_kind:
        result = PyType_GenericNew(BoolOp_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_boolop(o->v.BoolOp.op);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_op, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_list(o->v.BoolOp.values, ast2obj_expr);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_values, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case BinOp_kind:
        result = PyType_GenericNew(BinOp_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_expr(o->v.BinOp.left);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_left, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_operator(o->v.BinOp.op);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_op, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_expr(o->v.BinOp.right);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_right, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case UnaryOp_kind:
        result = PyType_GenericNew(UnaryOp_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_unaryop(o->v.UnaryOp.op);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_op, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_expr(o->v.UnaryOp.operand);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_operand, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case Lambda_kind:
        result = PyType_GenericNew(Lambda_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_arguments(o->v.Lambda.args);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_args, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_expr(o->v.Lambda.body);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_body, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case IfExp_kind:
        result = PyType_GenericNew(IfExp_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_expr(o->v.IfExp.test);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_test, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_expr(o->v.IfExp.body);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_body, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_expr(o->v.IfExp.orelse);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_orelse, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case Dict_kind:
        result = PyType_GenericNew(Dict_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_list(o->v.Dict.keys, ast2obj_expr);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_keys, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_list(o->v.Dict.values, ast2obj_expr);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_values, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case Set_kind:
        result = PyType_GenericNew(Set_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_list(o->v.Set.elts, ast2obj_expr);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_elts, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case ListComp_kind:
        result = PyType_GenericNew(ListComp_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_expr(o->v.ListComp.elt);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_elt, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_list(o->v.ListComp.generators, ast2obj_comprehension);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_generators, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case SetComp_kind:
        result = PyType_GenericNew(SetComp_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_expr(o->v.SetComp.elt);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_elt, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_list(o->v.SetComp.generators, ast2obj_comprehension);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_generators, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case DictComp_kind:
        result = PyType_GenericNew(DictComp_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_expr(o->v.DictComp.key);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_key, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_expr(o->v.DictComp.value);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_value, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_list(o->v.DictComp.generators, ast2obj_comprehension);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_generators, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case GeneratorExp_kind:
        result = PyType_GenericNew(GeneratorExp_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_expr(o->v.GeneratorExp.elt);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_elt, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_list(o->v.GeneratorExp.generators,
                             ast2obj_comprehension);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_generators, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case Await_kind:
        result = PyType_GenericNew(Await_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_expr(o->v.Await.value);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_value, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case Yield_kind:
        result = PyType_GenericNew(Yield_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_expr(o->v.Yield.value);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_value, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case YieldFrom_kind:
        result = PyType_GenericNew(YieldFrom_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_expr(o->v.YieldFrom.value);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_value, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case Compare_kind:
        result = PyType_GenericNew(Compare_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_expr(o->v.Compare.left);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_left, value) == -1)
            goto failed;
        Py_DECREF(value);
        {
            Py_ssize_t i, n = asdl_seq_LEN(o->v.Compare.ops);
            value = PyList_New(n);
            if (!value) goto failed;
            for(i = 0; i < n; i++)
                PyList_SET_ITEM(value, i, ast2obj_cmpop((cmpop_ty)asdl_seq_GET(o->v.Compare.ops, i)));
        }
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_ops, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_list(o->v.Compare.comparators, ast2obj_expr);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_comparators, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case Call_kind:
        result = PyType_GenericNew(Call_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_expr(o->v.Call.func);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_func, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_list(o->v.Call.args, ast2obj_expr);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_args, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_list(o->v.Call.keywords, ast2obj_keyword);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_keywords, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case Num_kind:
        result = PyType_GenericNew(Num_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_object(o->v.Num.n);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_n, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case Str_kind:
        result = PyType_GenericNew(Str_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_string(o->v.Str.s);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_s, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_string(o->v.Str.kind);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_kind, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case FormattedValue_kind:
        result = PyType_GenericNew(FormattedValue_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_expr(o->v.FormattedValue.value);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_value, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_int(o->v.FormattedValue.conversion);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_conversion, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_expr(o->v.FormattedValue.format_spec);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_format_spec, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case JoinedStr_kind:
        result = PyType_GenericNew(JoinedStr_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_list(o->v.JoinedStr.values, ast2obj_expr);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_values, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case Bytes_kind:
        result = PyType_GenericNew(Bytes_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_bytes(o->v.Bytes.s);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_s, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case NameConstant_kind:
        result = PyType_GenericNew(NameConstant_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_singleton(o->v.NameConstant.value);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_value, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case Ellipsis_kind:
        result = PyType_GenericNew(Ellipsis_type, NULL, NULL);
        if (!result) goto failed;
        break;
    case Constant_kind:
        result = PyType_GenericNew(Constant_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_constant(o->v.Constant.value);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_value, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case Attribute_kind:
        result = PyType_GenericNew(Attribute_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_expr(o->v.Attribute.value);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_value, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_identifier(o->v.Attribute.attr);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_attr, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_expr_context(o->v.Attribute.ctx);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_ctx, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case Subscript_kind:
        result = PyType_GenericNew(Subscript_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_expr(o->v.Subscript.value);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_value, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_slice(o->v.Subscript.slice);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_slice, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_expr_context(o->v.Subscript.ctx);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_ctx, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case Starred_kind:
        result = PyType_GenericNew(Starred_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_expr(o->v.Starred.value);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_value, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_expr_context(o->v.Starred.ctx);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_ctx, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case Name_kind:
        result = PyType_GenericNew(Name_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_identifier(o->v.Name.id);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_id, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_expr_context(o->v.Name.ctx);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_ctx, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case List_kind:
        result = PyType_GenericNew(List_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_list(o->v.List.elts, ast2obj_expr);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_elts, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_expr_context(o->v.List.ctx);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_ctx, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    case Tuple_kind:
        result = PyType_GenericNew(Tuple_type, NULL, NULL);
        if (!result) goto failed;
        value = ast2obj_list(o->v.Tuple.elts, ast2obj_expr);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_elts, value) == -1)
            goto failed;
        Py_DECREF(value);
        value = ast2obj_expr_context(o->v.Tuple.ctx);
        if (!value) goto failed;
        if (_PyObject_SetAttrId(result, &PyId_ctx, value) == -1)
            goto failed;
        Py_DECREF(value);
        break;
    }
    value = ast2obj_int(o->lineno);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_lineno, value) < 0)
        goto failed;
    Py_DECREF(value);
    value = ast2obj_int(o->col_offset);
    if (!value) goto failed;
    if (_PyObject_SetAttrId(result, &PyId_col_offset, value) < 0)
        goto failed;
    Py_DECREF(value);
    return result;
failed:
    Py_XDECREF(value);
    Py_XDECREF(result);
    return NULL;
}