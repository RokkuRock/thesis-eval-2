size_t mobi_get_attribute_value(char *value, const unsigned char *data, const size_t size, const char *attribute, bool only_quoted) {
    if (!data) {
        debug_print("Data is null%s", "\n");
        return SIZE_MAX;
    }
    size_t length = size;
    size_t attr_length = strlen(attribute);
    if (attr_length > MOBI_ATTRNAME_MAXSIZE) {
        debug_print("Attribute too long: %zu\n", attr_length);
        return SIZE_MAX;
    }
    char attr[MOBI_ATTRNAME_MAXSIZE + 2];
    strcpy(attr, attribute);
    strcat(attr, "=");
    attr_length++;
    if (size < attr_length) {
        return SIZE_MAX;
    }
    unsigned char last_border = '\0';
    do {
        if (*data == '<' || *data == '>') {
            last_border = *data;
        }
        if (length > attr_length + 1 && memcmp(data, attr, attr_length) == 0) {
            size_t offset = size - length;
            if (last_border == '>') {
                data += attr_length;
                length -= attr_length - 1;
                continue;
            }
            if (offset > 0) {
                if (data[-1] != '<' && !isspace(data[-1])) {
                    data += attr_length;
                    length -= attr_length - 1;
                    continue;
                }
            }
            data += attr_length;
            length -= attr_length;
            unsigned char separator;
            if (*data != '\'' && *data != '"') {
                if (only_quoted) {
                    continue;
                }
                separator = ' ';
            } else {
                separator = *data;
                data++;
                length--;
            }
            size_t j;
            for (j = 0; j < MOBI_ATTRVALUE_MAXSIZE && length && *data != separator && *data != '>'; j++) {
                *value++ = (char) *data++;
                length--;
            }
            if (*(data - 1) == '/' && *data == '>') {
                value--;
            }
            *value = '\0';
            return size - length - j;
        }
        data++;
    } while (--length);
    value[0] = '\0';
    return SIZE_MAX;
}