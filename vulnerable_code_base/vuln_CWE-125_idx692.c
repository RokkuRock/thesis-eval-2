Ta3AST_FromNode(const node *n, PyCompilerFlags *flags, const char *filename_str,
               int feature_version, PyArena *arena)
{
    mod_ty mod;
    PyObject *filename;
    filename = PyUnicode_DecodeFSDefault(filename_str);
    if (filename == NULL)
        return NULL;
    mod = Ta3AST_FromNodeObject(n, flags, filename, feature_version, arena);
    Py_DECREF(filename);
    return mod;
}