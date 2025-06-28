static int add_ast_fields(void)
{
    PyObject *empty_tuple, *d;
    if (PyType_Ready(&AST_type) < 0)
        return -1;
    d = AST_type.tp_dict;
    empty_tuple = PyTuple_New(0);
    if (!empty_tuple ||
        PyDict_SetItemString(d, "_fields", empty_tuple) < 0 ||
        PyDict_SetItemString(d, "_attributes", empty_tuple) < 0) {
        Py_XDECREF(empty_tuple);
        return -1;
    }
    Py_DECREF(empty_tuple);
    return 0;
}