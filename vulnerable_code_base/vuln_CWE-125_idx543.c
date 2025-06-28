new_identifier(const char *n, struct compiling *c)
{
    PyObject *id = PyUnicode_DecodeUTF8(n, strlen(n), NULL);
    if (!id)
        return NULL;
    assert(PyUnicode_IS_READY(id));
    if (!PyUnicode_IS_ASCII(id)) {
        PyObject *id2;
        if (!c->c_normalize && !init_normalization(c)) {
            Py_DECREF(id);
            return NULL;
        }
        PyTuple_SET_ITEM(c->c_normalize_args, 1, id);
        id2 = PyObject_Call(c->c_normalize, c->c_normalize_args, NULL);
        Py_DECREF(id);
        if (!id2)
            return NULL;
        id = id2;
    }
    PyUnicode_InternInPlace(&id);
    if (PyArena_AddPyObject(c->c_arena, id) < 0) {
        Py_DECREF(id);
        return NULL;
    }
    return id;
}