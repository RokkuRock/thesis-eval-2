_pdfioDictSetValue(
    pdfio_dict_t   *dict,		 
    const char     *key,		 
    _pdfio_value_t *value)		 
{
  _pdfio_pair_t	*pair;			 
  PDFIO_DEBUG("_pdfioDictSetValue(dict=%p, key=\"%s\", value=%p)\n", dict, key, (void *)value);
  if (dict->num_pairs > 0)
  {
    _pdfio_pair_t	pkey;		 
    pkey.key = key;
    if ((pair = (_pdfio_pair_t *)bsearch(&pkey, dict->pairs, dict->num_pairs, sizeof(_pdfio_pair_t), (int (*)(const void *, const void *))compare_pairs)) != NULL)
    {
      PDFIO_DEBUG("_pdfioDictSetValue: Replacing existing value.\n");
      if (pair->value.type == PDFIO_VALTYPE_BINARY)
        free(pair->value.value.binary.data);
      pair->value = *value;
      return (true);
    }
  }
  if (dict->num_pairs >= dict->alloc_pairs)
  {
    _pdfio_pair_t *temp = (_pdfio_pair_t *)realloc(dict->pairs, (dict->alloc_pairs + 8) * sizeof(_pdfio_pair_t));
    if (!temp)
    {
      PDFIO_DEBUG("_pdfioDictSetValue: Out of memory.\n");
      return (false);
    }
    dict->pairs       = temp;
    dict->alloc_pairs += 8;
  }
  pair = dict->pairs + dict->num_pairs;
  dict->num_pairs ++;
  pair->key   = key;
  pair->value = *value;
  if (dict->num_pairs > 1 && compare_pairs(pair - 1, pair) > 0)
    qsort(dict->pairs, dict->num_pairs, sizeof(_pdfio_pair_t), (int (*)(const void *, const void *))compare_pairs);
#ifdef DEBUG
  PDFIO_DEBUG("_pdfioDictSetValue(%p): %lu pairs\n", (void *)dict, (unsigned long)dict->num_pairs);
  PDFIO_DEBUG("_pdfioDictSetValue(%p): ", (void *)dict);
  PDFIO_DEBUG_DICT(dict);
  PDFIO_DEBUG("\n");
#endif  
  return (true);
}