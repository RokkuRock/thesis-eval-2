spnego_gss_init_sec_context(
			OM_uint32 *minor_status,
			gss_cred_id_t claimant_cred_handle,
			gss_ctx_id_t *context_handle,
			gss_name_t target_name,
			gss_OID mech_type,
			OM_uint32 req_flags,
			OM_uint32 time_req,
			gss_channel_bindings_t input_chan_bindings,
			gss_buffer_t input_token,
			gss_OID *actual_mech,
			gss_buffer_t output_token,
			OM_uint32 *ret_flags,
			OM_uint32 *time_rec)
{
	send_token_flag send_token = NO_TOKEN_SEND;
	OM_uint32 tmpmin, ret, negState;
	gss_buffer_t mechtok_in, mechListMIC_in, mechListMIC_out;
	gss_buffer_desc mechtok_out = GSS_C_EMPTY_BUFFER;
	spnego_gss_cred_id_t spcred = NULL;
	spnego_gss_ctx_id_t spnego_ctx = NULL;
	dsyslog("Entering init_sec_context\n");
	mechtok_in = mechListMIC_out = mechListMIC_in = GSS_C_NO_BUFFER;
	negState = REJECT;
	if (minor_status != NULL)
		*minor_status = 0;
	if (output_token != GSS_C_NO_BUFFER) {
		output_token->length = 0;
		output_token->value = NULL;
	}
	if (minor_status == NULL ||
	    output_token == GSS_C_NO_BUFFER ||
	    context_handle == NULL)
		return GSS_S_CALL_INACCESSIBLE_WRITE;
	if (actual_mech != NULL)
		*actual_mech = GSS_C_NO_OID;
	spcred = (spnego_gss_cred_id_t)claimant_cred_handle;
	if (*context_handle == GSS_C_NO_CONTEXT) {
		ret = init_ctx_new(minor_status, spcred,
				   context_handle, &send_token);
		if (ret != GSS_S_CONTINUE_NEEDED) {
			goto cleanup;
		}
	} else {
		ret = init_ctx_cont(minor_status, context_handle,
				    input_token, &mechtok_in,
				    &mechListMIC_in, &negState, &send_token);
		if (HARD_ERROR(ret)) {
			goto cleanup;
		}
	}
	spnego_ctx = (spnego_gss_ctx_id_t)*context_handle;
	if (!spnego_ctx->mech_complete) {
		ret = init_ctx_call_init(
			minor_status, spnego_ctx, spcred,
			target_name, req_flags,
			time_req, mechtok_in,
			actual_mech, &mechtok_out,
			ret_flags, time_rec,
			&negState, &send_token);
		if (!HARD_ERROR(ret) && mech_requires_mechlistMIC(spnego_ctx))
			spnego_ctx->mic_reqd = 1;
	}
	if (!HARD_ERROR(ret) && spnego_ctx->mech_complete &&
	    (spnego_ctx->ctx_flags & GSS_C_INTEG_FLAG)) {
		ret = handle_mic(minor_status,
				 mechListMIC_in,
				 (mechtok_out.length != 0),
				 spnego_ctx, &mechListMIC_out,
				 &negState, &send_token);
	}
cleanup:
	if (send_token == INIT_TOKEN_SEND) {
		if (make_spnego_tokenInit_msg(spnego_ctx,
					      0,
					      mechListMIC_out,
					      req_flags,
					      &mechtok_out, send_token,
					      output_token) < 0) {
			ret = GSS_S_FAILURE;
		}
	} else if (send_token != NO_TOKEN_SEND) {
		if (make_spnego_tokenTarg_msg(negState, GSS_C_NO_OID,
					      &mechtok_out, mechListMIC_out,
					      send_token,
					      output_token) < 0) {
			ret = GSS_S_FAILURE;
		}
	}
	gss_release_buffer(&tmpmin, &mechtok_out);
	if (ret == GSS_S_COMPLETE) {
		*context_handle = (gss_ctx_id_t)spnego_ctx->ctx_handle;
		if (actual_mech != NULL)
			*actual_mech = spnego_ctx->actual_mech;
		if (ret_flags != NULL)
			*ret_flags = spnego_ctx->ctx_flags;
		release_spnego_ctx(&spnego_ctx);
	} else if (ret != GSS_S_CONTINUE_NEEDED) {
		if (spnego_ctx != NULL) {
			gss_delete_sec_context(&tmpmin,
					       &spnego_ctx->ctx_handle,
					       GSS_C_NO_BUFFER);
			release_spnego_ctx(&spnego_ctx);
		}
		*context_handle = GSS_C_NO_CONTEXT;
	}
	if (mechtok_in != GSS_C_NO_BUFFER) {
		gss_release_buffer(&tmpmin, mechtok_in);
		free(mechtok_in);
	}
	if (mechListMIC_in != GSS_C_NO_BUFFER) {
		gss_release_buffer(&tmpmin, mechListMIC_in);
		free(mechListMIC_in);
	}
	if (mechListMIC_out != GSS_C_NO_BUFFER) {
		gss_release_buffer(&tmpmin, mechListMIC_out);
		free(mechListMIC_out);
	}
	return ret;
}  