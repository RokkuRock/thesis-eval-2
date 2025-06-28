void acpi_ns_terminate(void)
{
	acpi_status status;
	ACPI_FUNCTION_TRACE(ns_terminate);
#ifdef ACPI_EXEC_APP
	{
		union acpi_operand_object *prev;
		union acpi_operand_object *next;
		next = acpi_gbl_module_code_list;
		while (next) {
			prev = next;
			next = next->method.mutex;
			prev->method.mutex = NULL;	 
			acpi_ut_remove_reference(prev);
		}
	}
#endif
	acpi_ns_delete_namespace_subtree(acpi_gbl_root_node);
	status = acpi_ut_acquire_mutex(ACPI_MTX_NAMESPACE);
	if (ACPI_FAILURE(status)) {
		return_VOID;
	}
	acpi_ns_delete_node(acpi_gbl_root_node);
	(void)acpi_ut_release_mutex(ACPI_MTX_NAMESPACE);
	ACPI_DEBUG_PRINT((ACPI_DB_INFO, "Namespace freed\n"));
	return_VOID;
}