comics_check_decompress_command	(gchar          *mime_type, 
				 ComicsDocument *comics_document,
				 GError         **error)
{
	gboolean success;
	gchar *std_out, *std_err;
	gint retval;
	GError *err = NULL;
	if (g_content_type_is_a (mime_type, "application/x-cbr") ||
	    g_content_type_is_a (mime_type, "application/x-rar")) {
		comics_document->selected_command = 
					g_find_program_in_path ("unrar");
		if (comics_document->selected_command) {
			success = 
				g_spawn_command_line_sync (
				              comics_document->selected_command, 
							   &std_out, &std_err,
							   &retval, &err);
			if (!success) {
				g_propagate_error (error, err);
				g_error_free (err);
				return FALSE;
			} else if (WIFEXITED (retval)) {
				if (g_strrstr (std_out,"freeware") != NULL)
					comics_document->command_usage = RARLABS;
				else
					comics_document->command_usage = GNAUNRAR;
				g_free (std_out);
				g_free (std_err);
				return TRUE;
			}
		}
		comics_document->selected_command = 
				g_find_program_in_path ("unrar-free");
		if (comics_document->selected_command) {
			comics_document->command_usage = GNAUNRAR;
			return TRUE;
		}
		comics_document->selected_command =
				g_find_program_in_path ("bsdtar");
		if (comics_document->selected_command) {
			comics_document->command_usage = TAR;
			return TRUE;
		}
	} else if (g_content_type_is_a (mime_type, "application/x-cbz") ||
		   g_content_type_is_a (mime_type, "application/zip")) {
		comics_document->selected_command = 
				g_find_program_in_path ("unzip");
		comics_document->alternative_command =
				g_find_program_in_path ("zipnote");
		if (comics_document->selected_command &&
		    comics_document->alternative_command) {
			comics_document->command_usage = UNZIP;
			return TRUE;
		}
		comics_document->selected_command =
				g_find_program_in_path ("7za");
		if (comics_document->selected_command) {
			comics_document->command_usage = P7ZIP;
			return TRUE;
		}
		comics_document->selected_command =
				g_find_program_in_path ("7z");
		if (comics_document->selected_command) {
			comics_document->command_usage = P7ZIP;
			return TRUE;
		}
		comics_document->selected_command =
				g_find_program_in_path ("bsdtar");
		if (comics_document->selected_command) {
			comics_document->command_usage = TAR;
			return TRUE;
		}
	} else if (g_content_type_is_a (mime_type, "application/x-cb7") ||
		   g_content_type_is_a (mime_type, "application/x-7z-compressed")) {
		comics_document->selected_command =
				g_find_program_in_path ("7zr");
		if (comics_document->selected_command) {
			comics_document->command_usage = P7ZIP;
			return TRUE;
		}
		comics_document->selected_command =
				g_find_program_in_path ("7za");
		if (comics_document->selected_command) {
			comics_document->command_usage = P7ZIP;
			return TRUE;
		}
		comics_document->selected_command =
				g_find_program_in_path ("7z");
		if (comics_document->selected_command) {
			comics_document->command_usage = P7ZIP;
			return TRUE;
		}
		comics_document->selected_command =
				g_find_program_in_path ("bsdtar");
		if (comics_document->selected_command) {
			comics_document->command_usage = TAR;
			return TRUE;
		}
	} else if (g_content_type_is_a (mime_type, "application/x-cbt") ||
		   g_content_type_is_a (mime_type, "application/x-tar")) {
		comics_document->selected_command =
				g_find_program_in_path ("tar");
		if (comics_document->selected_command) {
			comics_document->command_usage = TAR;
			return TRUE;
		}
		comics_document->selected_command =
				g_find_program_in_path ("bsdtar");
		if (comics_document->selected_command) {
			comics_document->command_usage = TAR;
			return TRUE;
		}
	} else {
		g_set_error (error,
			     EV_DOCUMENT_ERROR,
			     EV_DOCUMENT_ERROR_INVALID,
			     _("Not a comic book MIME type: %s"),
			     mime_type);
			     return FALSE;
	}
	g_set_error_literal (error,
			     EV_DOCUMENT_ERROR,
			     EV_DOCUMENT_ERROR_INVALID,
			     _("Can���t find an appropriate command to "
			     "decompress this type of comic book"));
	return FALSE;
}