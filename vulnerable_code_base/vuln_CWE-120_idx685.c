START_TEST(test_log_long_msg)
{
	int lpc;
	int rc;
	int i, max = 1000;
	char *buffer = calloc(1, max);
	qb_log_init("test", LOG_USER, LOG_DEBUG);
	rc = qb_log_ctl(QB_LOG_SYSLOG, QB_LOG_CONF_ENABLED, QB_FALSE);
	ck_assert_int_eq(rc, 0);
	rc = qb_log_ctl(QB_LOG_BLACKBOX, QB_LOG_CONF_SIZE, 1024);
	ck_assert_int_eq(rc, 0);
	rc = qb_log_ctl(QB_LOG_BLACKBOX, QB_LOG_CONF_ENABLED, QB_TRUE);
	ck_assert_int_eq(rc, 0);
	rc = qb_log_filter_ctl(QB_LOG_BLACKBOX, QB_LOG_FILTER_ADD,
			  QB_LOG_FILTER_FILE, "*", LOG_TRACE);
	ck_assert_int_eq(rc, 0);
	for (lpc = 500; lpc < max; lpc++) {
		lpc++;
		for(i = 0; i < max; i++) {
			buffer[i] = 'a' + (i % 10);
		}
		buffer[lpc%600] = 0;
		qb_log(LOG_INFO, "Message %d %d - %s", lpc, lpc%600, buffer);
	}
        qb_log_blackbox_write_to_file("blackbox.dump");
        qb_log_blackbox_print_from_file("blackbox.dump");
	unlink("blackbox.dump");
	qb_log_fini();
}