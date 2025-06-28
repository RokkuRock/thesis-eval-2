minimask_equal(const struct minimask *a, const struct minimask *b)
{
    return !memcmp(a, b, sizeof *a
                   + MINIFLOW_VALUES_SIZE(miniflow_n_values(&a->masks)));
}