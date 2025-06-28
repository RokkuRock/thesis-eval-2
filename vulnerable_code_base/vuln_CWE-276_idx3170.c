static int samldb_spn_uniqueness_check(struct samldb_ctx *ac,
				       struct ldb_message_element *spn_el)
{
	struct ldb_context *ldb = ldb_module_get_ctx(ac->module);
	int ret;
	const char *spn = NULL;
	size_t i;
	TALLOC_CTX *tmp_ctx = talloc_new(ac->msg);
	if (tmp_ctx == NULL) {
		return ldb_oom(ldb);
	}
	for (i = 0; i < spn_el->num_values; i++) {
		int n_components;
		spn = (char *)spn_el->values[i].data;
		n_components = count_spn_components(spn_el->values[i]);
		if (n_components > 3 || n_components < 2) {
			ldb_asprintf_errstring(ldb,
					       "samldb: spn[%s] invalid with %u components",
					       spn, n_components);
			talloc_free(tmp_ctx);
			return LDB_ERR_CONSTRAINT_VIOLATION;
		}
		ret = check_spn_direct_collision(ldb,
						 tmp_ctx,
						 spn,
						 ac->msg->dn);
		if (ret == LDB_ERR_COMPARE_TRUE) {
			DBG_INFO("SPN %s re-added to the same object\n", spn);
			talloc_free(tmp_ctx);
			return LDB_SUCCESS;
		}
		if (ret != LDB_SUCCESS) {
			DBG_ERR("SPN %s failed direct uniqueness check\n", spn);
			talloc_free(tmp_ctx);
			return ret;
		}
		ret = check_spn_alias_collision(ldb,
						tmp_ctx,
						spn,
						ac->msg->dn);
		if (ret == LDB_ERR_NO_SUCH_OBJECT) {
			break;
		}
		if (ret != LDB_SUCCESS) {
			DBG_ERR("SPN %s failed alias uniqueness check\n", spn);
			talloc_free(tmp_ctx);
			return ret;
		}
		DBG_INFO("SPN %s seems to be unique\n", spn);
	}
	talloc_free(tmp_ctx);
	return LDB_SUCCESS;
}