int hashtable_set(hashtable_t *hashtable,
                  const char *key, size_t serial,
                  json_t *value)
{
    pair_t *pair;
    bucket_t *bucket;
    size_t hash, index;
    if(hashtable->size >= num_buckets(hashtable))
        if(hashtable_do_rehash(hashtable))
            return -1;
    hash = hash_str(key);
    index = hash % num_buckets(hashtable);
    bucket = &hashtable->buckets[index];
    pair = hashtable_find_pair(hashtable, bucket, key, hash);
    if(pair)
    {
        json_decref(pair->value);
        pair->value = value;
    }
    else
    {
        pair = jsonp_malloc(offsetof(pair_t, key) + strlen(key) + 1);
        if(!pair)
            return -1;
        pair->hash = hash;
        pair->serial = serial;
        strcpy(pair->key, key);
        pair->value = value;
        list_init(&pair->list);
        insert_to_bucket(hashtable, bucket, &pair->list);
        hashtable->size++;
    }
    return 0;
}