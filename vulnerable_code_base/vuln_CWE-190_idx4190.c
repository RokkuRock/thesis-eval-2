PyMemoTable_Set(PyMemoTable *self, PyObject *key, Py_ssize_t value)
{
    PyMemoEntry *entry;
    assert(key != NULL);
    entry = _PyMemoTable_Lookup(self, key);
    if (entry->me_key != NULL) {
        entry->me_value = value;
        return 0;
    }
    Py_INCREF(key);
    entry->me_key = key;
    entry->me_value = value;
    self->mt_used++;
    if (!(self->mt_used * 3 >= (self->mt_mask + 1) * 2))
        return 0;
    return _PyMemoTable_ResizeTable(self,
        (self->mt_used > 50000 ? 2 : 4) * self->mt_used);
}