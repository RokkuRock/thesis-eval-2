MOBI_RET mobi_find_attrvalue(MOBIResult *result, const unsigned char *data_start, const unsigned char *data_end, const MOBIFiletype type, const char *needle) {
    if (!result) {
        debug_print("Result structure is null%s", "\n");
        return MOBI_PARAM_ERR;
    }
    result->start = result->end = NULL;
    *(result->value) = '\0';
    if (!data_start || !data_end) {
        debug_print("Data is null%s", "\n");
        return MOBI_PARAM_ERR;
    }
    size_t needle_length = strlen(needle);
    if (needle_length > MOBI_ATTRNAME_MAXSIZE) {
        debug_print("Attribute too long: %zu\n", needle_length);
        return MOBI_PARAM_ERR;
    }
    if (data_start + needle_length > data_end) {
        return MOBI_SUCCESS;
    }
    unsigned char *data = (unsigned char *) data_start;
    unsigned char tag_open;
    unsigned char tag_close;
    if (type == T_CSS) {
        tag_open = '{';
        tag_close = '}';
    } else {
        tag_open = '<';
        tag_close = '>';
    }
    unsigned char last_border = tag_close;
    while (data <= data_end) {
        if (*data == tag_open || *data == tag_close) {
            last_border = *data;
        }
        if (data + needle_length <= data_end && memcmp(data, needle, needle_length) == 0) {
            if (last_border != tag_open) {
                data += needle_length;
                continue;
            }
            while (data >= data_start && !isspace(*data) && *data != tag_open && *data != '=' && *data != '(') {
                data--;
            }
            result->is_url = (*data == '(');
            result->start = ++data;
            int i = 0;
            while (data <= data_end && !isspace(*data) && *data != tag_close && *data != ')' && i < MOBI_ATTRVALUE_MAXSIZE) {
                result->value[i++] = (char) *data++;
            }
            if (*(data - 1) == '/' && *data == '>') {
                --data; --i;
            }
            result->end = data;
            result->value[i] = '\0';
            return MOBI_SUCCESS;
        }
        data++;
    }
    return MOBI_SUCCESS;
}