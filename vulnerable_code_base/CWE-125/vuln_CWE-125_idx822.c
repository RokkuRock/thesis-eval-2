init_normalization(struct compiling *c)
{
    PyObject *m = PyImport_ImportModuleNoBlock("unicodedata");
    if (!m)
        return 0;
    c->c_normalize = PyObject_GetAttrString(m, "normalize");
    Py_DECREF(m);
    if (!c->c_normalize)
        return 0;
    c->c_normalize_args = Py_BuildValue("(sN)", "NFKC", Py_None);
    if (!c->c_normalize_args) {
        Py_CLEAR(c->c_normalize);
        return 0;
    }
    PyTuple_SET_ITEM(c->c_normalize_args, 1, NULL);
    return 1;
}