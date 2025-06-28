_PyMemoTable_ResizeTable(PyMemoTable *self, Py_ssize_t min_size)
{
    PyMemoEntry *oldtable = NULL;
    PyMemoEntry *oldentry, *newentry;
    Py_ssize_t new_size = MT_MINSIZE;
    Py_ssize_t to_process;
    assert(min_size > 0);
    while (new_size < min_size && new_size > 0)
        new_size <<= 1;
    if (new_size <= 0) {
        PyErr_NoMemory();
        return -1;
    }
    assert((new_size & (new_size - 1)) == 0);
    oldtable = self->mt_table;
    self->mt_table = PyMem_NEW(PyMemoEntry, new_size);
    if (self->mt_table == NULL) {
        self->mt_table = oldtable;
        PyErr_NoMemory();
        return -1;
    }
    self->mt_allocated = new_size;
    self->mt_mask = new_size - 1;
    memset(self->mt_table, 0, sizeof(PyMemoEntry) * new_size);
    to_process = self->mt_used;
    for (oldentry = oldtable; to_process > 0; oldentry++) {
        if (oldentry->me_key != NULL) {
            to_process--;
            newentry = _PyMemoTable_Lookup(self, oldentry->me_key);
            newentry->me_key = oldentry->me_key;
            newentry->me_value = oldentry->me_value;
        }
    }
    PyMem_FREE(oldtable);
    return 0;
}