static void cleanup_shm_refs(const uint64_t *saved_attr,
			     struct tee_ta_param *param, uint32_t num_params)
{
	size_t n;
	for (n = 0; n < num_params; n++) {
		switch (saved_attr[n]) {
		case OPTEE_MSG_ATTR_TYPE_TMEM_INPUT:
		case OPTEE_MSG_ATTR_TYPE_TMEM_OUTPUT:
		case OPTEE_MSG_ATTR_TYPE_TMEM_INOUT:
#ifdef CFG_CORE_DYN_SHM
		case OPTEE_MSG_ATTR_TYPE_RMEM_INPUT:
		case OPTEE_MSG_ATTR_TYPE_RMEM_OUTPUT:
		case OPTEE_MSG_ATTR_TYPE_RMEM_INOUT:
#endif
			mobj_put(param->u[n].mem.mobj);
			break;
		default:
			break;
		}
	}
}