PyMemoTable_Copy(PyMemoTable *self)
{
    Py_ssize_t i;
    PyMemoTable *new = PyMemoTable_New();
    if (new == NULL)
        return NULL;
    new->mt_used = self->mt_used;
    new->mt_allocated = self->mt_allocated;
    new->mt_mask = self->mt_mask;
    PyMem_FREE(new->mt_table);
    new->mt_table = PyMem_NEW(PyMemoEntry, self->mt_allocated);
    if (new->mt_table == NULL) {
        PyMem_FREE(new);
        PyErr_NoMemory();
        return NULL;
    }
    for (i = 0; i < self->mt_allocated; i++) {
        Py_XINCREF(self->mt_table[i].me_key);
    }
    memcpy(new->mt_table, self->mt_table,
           sizeof(PyMemoEntry) * self->mt_allocated);
    return new;
}