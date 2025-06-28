static void test_sss_certmap_get_search_filter(void **state)
{
    int ret;
    struct sss_certmap_ctx *ctx;
    char *filter;
    char **domains;
    const char *dom_list[] = {"test.dom", NULL};
    ret = sss_certmap_init(NULL, ext_debug, NULL, &ctx);
    assert_int_equal(ret, EOK);
    assert_non_null(ctx);
    assert_null(ctx->prio_list);
    ret = sss_certmap_add_rule(ctx, 100,
                            "KRB5:<ISSUER>CN=Certificate Authority,O=IPA.DEVEL",
                            "LDAP:rule100=<I>{issuer_dn}<S>{subject_dn}", NULL);
    assert_int_equal(ret, 0);
    ret = sss_certmap_get_search_filter(ctx, discard_const(test_cert_der),
                                        sizeof(test_cert_der),
                                        &filter, &domains);
    assert_int_equal(ret, 0);
    assert_non_null(filter);
    assert_string_equal(filter, "rule100=<I>CN=Certificate Authority,O=IPA.DEVEL"
                                "<S>CN=ipa-devel.ipa.devel,O=IPA.DEVEL");
    assert_null(domains);
    ret = sss_certmap_add_rule(ctx, 99,
                            "KRB5:<ISSUER>CN=Certificate Authority,O=IPA.DEVEL",
                            "LDAP:rule99=<I>{issuer_dn}<S>{subject_dn}",
                            dom_list);
    assert_int_equal(ret, 0);
    ret = sss_certmap_get_search_filter(ctx, discard_const(test_cert_der),
                                        sizeof(test_cert_der),
                                        &filter, &domains);
    assert_int_equal(ret, 0);
    assert_non_null(filter);
    assert_string_equal(filter, "rule99=<I>CN=Certificate Authority,O=IPA.DEVEL"
                                "<S>CN=ipa-devel.ipa.devel,O=IPA.DEVEL");
    assert_non_null(domains);
    assert_string_equal(domains[0], "test.dom");
    assert_null(domains[1]);
    ret = sss_certmap_add_rule(ctx, 98,
                            "KRB5:<ISSUER>CN=Certificate Authority,O=IPA.DEVEL",
                            "LDAP:rule98=userCertificate;binary={cert!bin}",
                            dom_list);
    assert_int_equal(ret, 0);
    ret = sss_certmap_get_search_filter(ctx, discard_const(test_cert_der),
                                        sizeof(test_cert_der),
                                        &filter, &domains);
    assert_int_equal(ret, 0);
    assert_non_null(filter);
    assert_string_equal(filter, "rule98=userCertificate;binary=" TEST_CERT_BIN);
    assert_non_null(domains);
    assert_string_equal(domains[0], "test.dom");
    assert_null(domains[1]);
    ret = sss_certmap_add_rule(ctx, 97,
                            "KRB5:<ISSUER>CN=Certificate Authority,O=IPA.DEVEL",
                            "LDAP:rule97=<I>{issuer_dn!nss_x500}<S>{subject_dn}",
                            dom_list);
    assert_int_equal(ret, 0);
    ret = sss_certmap_get_search_filter(ctx, discard_const(test_cert_der),
                                        sizeof(test_cert_der),
                                        &filter, &domains);
    assert_int_equal(ret, 0);
    assert_non_null(filter);
    assert_string_equal(filter, "rule97=<I>O=IPA.DEVEL,CN=Certificate Authority"
                                "<S>CN=ipa-devel.ipa.devel,O=IPA.DEVEL");
    assert_non_null(domains);
    assert_string_equal(domains[0], "test.dom");
    assert_null(domains[1]);
    ret = sss_certmap_add_rule(ctx, 96,
                            "KRB5:<ISSUER>CN=Certificate Authority,O=IPA.DEVEL",
                            "LDAP:rule96=<I>{issuer_dn!nss_x500}<S>{subject_dn!nss_x500}",
                            dom_list);
    assert_int_equal(ret, 0);
    ret = sss_certmap_get_search_filter(ctx, discard_const(test_cert_der),
                                        sizeof(test_cert_der),
                                        &filter, &domains);
    assert_int_equal(ret, 0);
    assert_non_null(filter);
    assert_string_equal(filter, "rule96=<I>O=IPA.DEVEL,CN=Certificate Authority"
                                "<S>O=IPA.DEVEL,CN=ipa-devel.ipa.devel");
    assert_non_null(domains);
    assert_string_equal(domains[0], "test.dom");
    assert_null(domains[1]);
    ret = sss_certmap_add_rule(ctx, 95,
                            "KRB5:<ISSUER>CN=Certificate Authority,O=IPA.DEVEL",
                            NULL, NULL);
    assert_int_equal(ret, 0);
    ret = sss_certmap_get_search_filter(ctx, discard_const(test_cert_der),
                                        sizeof(test_cert_der),
                                        &filter, &domains);
    assert_int_equal(ret, 0);
    assert_non_null(filter);
    assert_string_equal(filter, "(userCertificate;binary=" TEST_CERT_BIN ")");
    assert_null(domains);
    ret = sss_certmap_add_rule(ctx, 94,
                      "KRB5:<ISSUER>CN=Certificate Authority,O=IPA.DEVEL",
                      "LDAP:rule94=<I>{issuer_dn!ad_x500}<S>{subject_dn!ad_x500}",
                            dom_list);
    assert_int_equal(ret, 0);
    ret = sss_certmap_get_search_filter(ctx, discard_const(test_cert_der),
                                        sizeof(test_cert_der),
                                        &filter, &domains);
    assert_int_equal(ret, 0);
    assert_non_null(filter);
    assert_string_equal(filter, "rule94=<I>O=IPA.DEVEL,CN=Certificate Authority"
                                "<S>O=IPA.DEVEL,CN=ipa-devel.ipa.devel");
    assert_non_null(domains);
    assert_string_equal(domains[0], "test.dom");
    assert_null(domains[1]);
    ret = sss_certmap_add_rule(ctx, 89, NULL,
                            "(rule89={subject_nt_principal})",
                            NULL);
    assert_int_equal(ret, 0);
    ret = sss_certmap_get_search_filter(ctx, discard_const(test_cert2_der),
                                        sizeof(test_cert2_der),
                                        &filter, &domains);
    assert_int_equal(ret, 0);
    assert_non_null(filter);
    assert_string_equal(filter, "(rule89=tu1@ad.devel)");
    assert_null(domains);
    ret = sss_certmap_add_rule(ctx, 88, NULL,
                            "(rule88={subject_nt_principal.short_name})",
                            NULL);
    assert_int_equal(ret, 0);
    ret = sss_certmap_get_search_filter(ctx, discard_const(test_cert2_der),
                                        sizeof(test_cert2_der),
                                        &filter, &domains);
    assert_int_equal(ret, 0);
    assert_non_null(filter);
    assert_string_equal(filter, "(rule88=tu1)");
    assert_null(domains);
    ret = sss_certmap_add_rule(ctx, 87, NULL,
                          "LDAP:rule87=<I>{issuer_dn!nss_x500}<S>{subject_dn!nss_x500}",
                          NULL);
    assert_int_equal(ret, 0);
    ret = sss_certmap_get_search_filter(ctx, discard_const(test_cert2_der),
                                        sizeof(test_cert2_der),
                                        &filter, &domains);
    assert_int_equal(ret, 0);
    assert_non_null(filter);
    assert_string_equal(filter, "rule87=<I>DC=devel,DC=ad,CN=ad-AD-SERVER-CA"
                  "<S>DC=devel,DC=ad,CN=Users,CN=t u,E=test.user@email.domain");
    assert_null(domains);
    ret = sss_certmap_add_rule(ctx, 86, NULL,
                      "LDAP:rule86=<I>{issuer_dn!ad_x500}<S>{subject_dn!ad_x500}",
                      NULL);
    assert_int_equal(ret, 0);
    ret = sss_certmap_get_search_filter(ctx, discard_const(test_cert2_der),
                                        sizeof(test_cert2_der),
                                        &filter, &domains);
    assert_int_equal(ret, 0);
    assert_non_null(filter);
    assert_string_equal(filter, "rule86=<I>DC=devel,DC=ad,CN=ad-AD-SERVER-CA"
                  "<S>DC=devel,DC=ad,CN=Users,CN=t u,E=test.user@email.domain");
    assert_null(domains);
    sss_certmap_free_ctx(ctx);
    ret = sss_certmap_init(NULL, ext_debug, NULL, &ctx);
    assert_int_equal(ret, EOK);
    assert_non_null(ctx);
    assert_null(ctx->prio_list);
    ret = sss_certmap_get_search_filter(ctx, discard_const(test_cert2_der),
                                        sizeof(test_cert2_der),
                                        &filter, &domains);
    assert_int_equal(ret, 0);
    assert_non_null(filter);
    assert_string_equal(filter, "(userCertificate;binary=" TEST_CERT2_BIN")");
    assert_null(domains);
    sss_certmap_free_ctx(ctx);
}