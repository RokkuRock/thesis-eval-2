int ec_glob(const char *pattern, const char *string)
{
    size_t                    i;
    int_pair *                p;
    char *                    c;
    char                      pcre_str[2 * PATTERN_MAX] = "^";
    char *                    p_pcre;
    char *                    pcre_str_end;
    int                       brace_level = 0;
    _Bool                     is_in_bracket = 0;
    int                       error_code;
    size_t                    erroffset;
    pcre2_code *              re;
    int                       rc;
    size_t *                  pcre_result;
    pcre2_match_data *        pcre_match_data;
    char                      l_pattern[2 * PATTERN_MAX];
    _Bool                     are_braces_paired = 1;
    UT_array *                nums;      
    int                       ret = 0;
    strcpy(l_pattern, pattern);
    p_pcre = pcre_str + 1;
    pcre_str_end = pcre_str + 2 * PATTERN_MAX;
    {
        int     left_count = 0;
        int     right_count = 0;
        for (c = l_pattern; *c; ++ c)
        {
            if (*c == '\\' && *(c+1) != '\0')
            {
                ++ c;
                continue;
            }
            if (*c == '}')
                ++ right_count;
            else if (*c == '{')
                ++ left_count;
            if (right_count > left_count)
            {
                are_braces_paired = 0;
                break;
            }
        }
        if (right_count != left_count)
            are_braces_paired = 0;
    }
    re = pcre2_compile("^\\{[\\+\\-]?\\d+\\.\\.[\\+\\-]?\\d+\\}$", PCRE2_ZERO_TERMINATED, 0,
            &error_code, &erroffset, NULL);
    if (!re)         
        return -1;
    utarray_new(nums, &ut_int_pair_icd);
    for (c = l_pattern; *c; ++ c)
    {
        switch (*c)
        {
        case '\\':       
            if (*(c+1) != '\0')
            {
                *(p_pcre ++) = *(c++);
                *(p_pcre ++) = *c;
            }
            else
                STRING_CAT(p_pcre, "\\\\", pcre_str_end);
            break;
        case '?':
            STRING_CAT(p_pcre, "[^/]", pcre_str_end);
            break;
        case '*':
            if (*(c+1) == '*')       
            {
                STRING_CAT(p_pcre, ".*", pcre_str_end);
                ++ c;
            }
            else                     
                STRING_CAT(p_pcre, "[^\\/]*", pcre_str_end);
            break;
        case '[':
            if (is_in_bracket)      
            {
                STRING_CAT(p_pcre, "\\[", pcre_str_end);
                break;
            }
            {
                _Bool           has_slash = 0;
                char *          cc;
                for (cc = c; *cc && *cc != ']'; ++ cc)
                {
                    if (*cc == '\\' && *(cc+1) != '\0')
                    {
                        ++ cc;
                        continue;
                    }
                    if (*cc == '/')
                    {
                        has_slash = 1;
                        break;
                    }
                }
                if (has_slash)
                {
                    char *           right_bracket = strchr(c, ']');
                    if (!right_bracket)   
                        right_bracket = c + strlen(c);
                    strcat(p_pcre, "\\");
                    strncat(p_pcre, c, right_bracket - c);
                    if (*right_bracket)   
                        strcat(p_pcre, "\\]");
                    p_pcre += strlen(p_pcre);
                    c = right_bracket;
                    if (!*c)
                        c -= 1;
                    break;
                }
            }
            is_in_bracket = 1;
            if (*(c+1) == '!')      
            {
                STRING_CAT(p_pcre, "[^", pcre_str_end);
                ++ c;
            }
            else
                *(p_pcre ++) = '[';
            break;
        case ']':
            is_in_bracket = 0;
            *(p_pcre ++) = *c;
            break;
        case '-':
            if (is_in_bracket)       
                *(p_pcre ++) = *c;
            else
                STRING_CAT(p_pcre, "\\-", pcre_str_end);
            break;
        case '{':
            if (!are_braces_paired)
            {
                STRING_CAT(p_pcre, "\\{", pcre_str_end);
                break;
            }
            {
                char *                   cc;
                _Bool                    is_single = 1;
                for (cc = c + 1; *cc != '\0' && *cc != '}'; ++ cc)
                {
                    if (*cc == '\\' && *(cc+1) != '\0')
                    {
                        ++ cc;
                        continue;
                    }
                    if (*cc == ',')
                    {
                        is_single = 0;
                        break;
                    }
                }
                if (*cc == '\0')
                    is_single = 0;
                if (is_single)       
                {
                    const char *        double_dots;
                    int_pair            pair;
                    pcre2_match_data *  match_data = pcre2_match_data_create_from_pattern(re, NULL);
                    rc = pcre2_match(re, c, cc - c + 1, 0, 0, match_data, NULL);
                    pcre2_match_data_free(match_data);
                    if (rc < 0)     
                    {
                        STRING_CAT(p_pcre, "\\{", pcre_str_end);
                        memmove(cc+1, cc, strlen(cc) + 1);
                        *cc = '\\';
                        break;
                    }
                    double_dots = strstr(c, "..");
                    pair.num1 = ec_atoi(c + 1);
                    pair.num2 = ec_atoi(double_dots + 2);
                    utarray_push_back(nums, &pair);
                    STRING_CAT(p_pcre, "([\\+\\-]?\\d+)", pcre_str_end);
                    c = cc;
                    break;
                }
            }
            ++ brace_level;
            STRING_CAT(p_pcre, "(?:", pcre_str_end);
            break;
        case '}':
            if (!are_braces_paired)
            {
                STRING_CAT(p_pcre, "\\}", pcre_str_end);
                break;
            }
            -- brace_level;
            *(p_pcre ++) = ')';
            break;
        case ',':
            if (brace_level > 0)   
                *(p_pcre ++) = '|';
            else
                STRING_CAT(p_pcre, "\\,", pcre_str_end);
            break;
        case '/':
            if (!strncmp(c, "/**/", 4))
            {
                STRING_CAT(p_pcre, "(\\/|\\/.*\\/)", pcre_str_end);
                c += 3;
            }
            else
                STRING_CAT(p_pcre, "\\/", pcre_str_end);
            break;
        default:
            if (!isalnum(*c))
                *(p_pcre ++) = '\\';
            *(p_pcre ++) = *c;
        }
    }
    *(p_pcre ++) = '$';
    pcre2_code_free(re);  
    re = pcre2_compile(pcre_str, PCRE2_ZERO_TERMINATED, 0, &error_code, &erroffset, NULL);
    if (!re)         
    {
        utarray_free(nums);
        return -1;
    }
    pcre_match_data = pcre2_match_data_create_from_pattern(re, NULL);
    rc = pcre2_match(re, string, strlen(string), 0, 0, pcre_match_data, NULL);
    if (rc < 0)      
    {
        if (rc == PCRE2_ERROR_NOMATCH)
            ret = EC_GLOB_NOMATCH;
        else
            ret = rc;
        goto cleanup;
    }
    pcre_result = pcre2_get_ovector_pointer(pcre_match_data);
    for(p = (int_pair *) utarray_front(nums), i = 1; p;
            ++ i, p = (int_pair *) utarray_next(nums, p))
    {
        const char * substring_start = string + pcre_result[2 * i];
        size_t  substring_length = pcre_result[2 * i + 1] - pcre_result[2 * i];
        char *       num_string;
        int          num;
        if (*substring_start == '0')
            break;
        num_string = strndup(substring_start, substring_length);
        if (num_string == NULL) {
          ret = -2;
          goto cleanup;
        }
        num = ec_atoi(num_string);
        free(num_string);
        if (num < p->num1 || num > p->num2)  
            break;
    }
    if (p != NULL)       
        ret = EC_GLOB_NOMATCH;
 cleanup:
    pcre2_code_free(re);
    pcre2_match_data_free(pcre_match_data);
    utarray_free(nums);
    return ret;
}