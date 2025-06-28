test_js (void) {
    GString *result = g_string_new("");
    parse_cmd_line("js ('x' + 345).toUpperCase()", result);
    g_assert_cmpstr("X345", ==, result->str);
    uzbl.net.useragent = "Test useragent";
    parse_cmd_line("js Uzbl.run('print @useragent').toUpperCase();", result);
    g_assert_cmpstr("TEST USERAGENT", ==, result->str);
    g_string_free(result, TRUE);
}