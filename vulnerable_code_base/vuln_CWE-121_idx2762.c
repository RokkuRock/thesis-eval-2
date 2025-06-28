gpointer parse_commands(gpointer user_data)
{
    fflush(stdout);
    char buf[BUFSIZE];
    while (1)
    {
        printf("> ");
        fflush(stdout);
        scanf("%s", buf);
        if (strcmp(buf, "stop") == 0)
        {
            cpdbDeleteFrontendObj(f);
            g_message("Stopping front end..\n");
            exit(0);
        }
        else if (strcmp(buf, "restart") == 0)
        {
            cpdbDisconnectFromDBus(f);
            cpdbConnectToDBus(f);
        }
        else if (strcmp(buf, "hide-remote") == 0)
        {
            cpdbHideRemotePrinters(f);
            g_message("Hiding remote printers discovered by the backend..\n");
        }
        else if (strcmp(buf, "unhide-remote") == 0)
        {
            cpdbUnhideRemotePrinters(f);
            g_message("Unhiding remote printers discovered by the backend..\n");
        }
        else if (strcmp(buf, "hide-temporary") == 0)
        {
            cpdbHideTemporaryPrinters(f);
            g_message("Hiding remote printers discovered by the backend..\n");
        }
        else if (strcmp(buf, "unhide-temporary") == 0)
        {
            cpdbUnhideTemporaryPrinters(f);
            g_message("Unhiding remote printers discovered by the backend..\n");
        }
        else if (strcmp(buf, "get-all-options") == 0)
        {
            char printer_id[BUFSIZE];
            char backend_name[BUFSIZE];
            scanf("%s%s", printer_id, backend_name);
            g_message("Getting all attributes ..\n");
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            if(p == NULL)
              continue;
            cpdb_options_t *opts = cpdbGetAllOptions(p);
            printf("Retrieved %d options.\n", opts->count);
            GHashTableIter iter;
            gpointer value;
            g_hash_table_iter_init(&iter, opts->table);
            while (g_hash_table_iter_next(&iter, NULL, &value))
            {
                printOption(value);
            }
        }
        else if (strcmp(buf, "get-all-media") == 0)
        {
            char printer_id[BUFSIZE];
            char backend_name[BUFSIZE];
            scanf("%s%s", printer_id, backend_name);
            g_message("Getting all attributes ..\n");
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            if(p == NULL)
              continue;
            cpdb_options_t *opts = cpdbGetAllOptions(p);
            printf("Retrieved %d medias.\n", opts->media_count);
            GHashTableIter iter;
            gpointer value;
            g_hash_table_iter_init(&iter, opts->media);
            while (g_hash_table_iter_next(&iter, NULL, &value))
            {
                printMedia(value);
            }
        }
        else if (strcmp(buf, "get-default") == 0)
        {
            char printer_id[BUFSIZE], backend_name[BUFSIZE], option_name[BUFSIZE];
            scanf("%s%s%s", option_name, printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            char *ans = cpdbGetDefault(p, option_name);
            if (!ans)
                printf("cpdb_option_t %s doesn't exist.", option_name);
            else
                printf("Default : %s\n", ans);
        }
        else if (strcmp(buf, "get-setting") == 0)
        {
            char printer_id[BUFSIZE], backend_name[BUFSIZE], setting_name[BUFSIZE];
            scanf("%s%s%s", setting_name, printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            char *ans = cpdbGetSetting(p, setting_name);
            if (!ans)
                printf("Setting %s doesn't exist.\n", setting_name);
            else
                printf("Setting value : %s\n", ans);
        }
        else if (strcmp(buf, "get-current") == 0)
        {
            char printer_id[BUFSIZE], backend_name[BUFSIZE], option_name[BUFSIZE];
            scanf("%s%s%s", option_name, printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            char *ans = cpdbGetCurrent(p, option_name);
            if (!ans)
                printf("cpdb_option_t %s doesn't exist.", option_name);
            else
                printf("Current value : %s\n", ans);
        }
        else if (strcmp(buf, "add-setting") == 0)
        {
            char printer_id[BUFSIZE], backend_name[BUFSIZE], option_name[BUFSIZE], option_val[BUFSIZE];
            scanf("%s %s %s %s", option_name, option_val, printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            printf("%s : %s\n", option_name, option_val);
            cpdbAddSettingToPrinter(p, cpdbGetStringCopy(option_name), cpdbGetStringCopy(option_val));
        }
        else if (strcmp(buf, "clear-setting") == 0)
        {
            char printer_id[BUFSIZE], backend_name[BUFSIZE], option_name[BUFSIZE];
            scanf("%s%s%s", option_name, printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            cpdbClearSettingFromPrinter(p, option_name);
        }
        else if (strcmp(buf, "get-state") == 0)
        {
            char printer_id[BUFSIZE];
            char backend_name[BUFSIZE];
            scanf("%s%s", printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            printf("%s\n", cpdbGetState(p));
        }
        else if (strcmp(buf, "is-accepting-jobs") == 0)
        {
            char printer_id[BUFSIZE];
            char backend_name[BUFSIZE];
            scanf("%s%s", printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            printf("Accepting jobs ? : %d \n", cpdbIsAcceptingJobs(p));
        }
        else if (strcmp(buf, "help") == 0)
        {
            display_help();
        }
        else if (strcmp(buf, "ping") == 0)
        {
            char printer_id[BUFSIZE], backend_name[BUFSIZE];
            scanf("%s%s", printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            print_backend_call_ping_sync(p->backend_proxy, p->id, NULL, NULL);
        }
        else if (strcmp(buf, "get-default-printer") == 0)
        {
            cpdb_printer_obj_t *p = cpdbGetDefaultPrinter(f);
            if (p)
                printf("%s#%s\n", p->name, p->backend_name);
            else
                printf("No default printer found\n");
        }
        else if (strcmp(buf, "get-default-printer-for-backend") == 0)
        {
            char backend_name[BUFSIZE];
            scanf("%s", backend_name);
            cpdb_printer_obj_t *p = cpdbGetDefaultPrinterForBackend(f, backend_name);
            printf("%s\n", p->name);
        }
        else if (strcmp(buf, "set-user-default-printer") == 0)
        {
            char printer_id[BUFSIZE];
            char backend_name[BUFSIZE];
            scanf("%s%s", printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            if (p)
            {
                if (cpdbSetUserDefaultPrinter(p))
                    printf("Set printer as user default\n");
                else
                    printf("Couldn't set printer as user default\n");
            }
        }
        else if (strcmp(buf, "set-system-default-printer") == 0)
        {
            char printer_id[BUFSIZE];
            char backend_name[BUFSIZE];
            scanf("%s%s", printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            if (p)
            {
                if (cpdbSetSystemDefaultPrinter(p))
                    printf("Set printer as system default\n");
                else
                    printf("Couldn't set printer as system default\n");
            }
        }
        else if (strcmp(buf, "print-file") == 0)
        {
            char printer_id[BUFSIZE], backend_name[BUFSIZE], file_path[BUFSIZE];
            scanf("%s%s%s", file_path, printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            if(strcmp(backend_name, "FILE") == 0)
            {
              char final_file_path[BUFSIZE];
              printf("Please give the final file path: ");
              scanf("%s", final_file_path);
              cpdbPrintFilePath(p, file_path, final_file_path);
              continue;
            }
            cpdbAddSettingToPrinter(p, "copies", "3");
            cpdbPrintFile(p, file_path);
        }
        else if (strcmp(buf, "get-active-jobs-count") == 0)
        {
            char printer_id[BUFSIZE];
            char backend_name[BUFSIZE];
            scanf("%s%s", printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            printf("%d jobs currently active.\n", cpdbGetActiveJobsCount(p));
        }
        else if (strcmp(buf, "get-all-jobs") == 0)
        {
            int active_only;
            scanf("%d", &active_only);
            cpdb_job_t *j;
            int x = cpdbGetAllJobs(f, &j, active_only);
            printf("Total %d jobs\n", x);
            int i;
            for (i = 0; i < x; i++)
            {
                printf("%s .. %s  .. %s  .. %s  .. %s\n", j[i].job_id, j[i].title, j[i].printer_id, j[i].state, j[i].submitted_at);
            }
        }
        else if (strcmp(buf, "cancel-job") == 0)
        {
            char printer_id[BUFSIZE];
            char backend_name[BUFSIZE];
            char job_id[BUFSIZE];
            scanf("%s%s%s", job_id, printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            if (cpdbCancelJob(p, job_id))
                printf("cpdb_job_t %s has been cancelled.\n", job_id);
            else
                printf("Unable to cancel job %s\n", job_id);
        }
        else if (strcmp(buf, "pickle-printer") == 0)
        {
            char printer_id[BUFSIZE];
            char backend_name[BUFSIZE];
            char job_id[BUFSIZE];
            scanf("%s%s", printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            cpdbPicklePrinterToFile(p, "/tmp/.printer-pickle", f);
        }
        else if (strcmp(buf, "get-option-translation") == 0)
        {
            char printer_id[BUFSIZE];
            char backend_name[BUFSIZE];
            char option_name[BUFSIZE];
            scanf("%s%s%s", option_name, printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            printf("%s\n", cpdbGetOptionTranslation(p, option_name, locale));
        }
        else if (strcmp(buf, "get-choice-translation") == 0)
        {
            char printer_id[BUFSIZE];
            char backend_name[BUFSIZE];
            char option_name[BUFSIZE];
            char choice_name[BUFSIZE];
            scanf("%s%s%s%s", option_name, choice_name, printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            printf("%s\n", cpdbGetChoiceTranslation(p, option_name, choice_name, locale));
        }
        else if (strcmp(buf, "get-group-translation") == 0)
        {
            char printer_id[BUFSIZE];
            char backend_name[BUFSIZE];
            char group_name[BUFSIZE];
            scanf("%s%s%s", group_name, printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            printf("%s\n", cpdbGetGroupTranslation(p, group_name, locale));
        }
        else if (strcmp(buf, "get-all-translations") == 0)
        {
            char printer_id[BUFSIZE];
            char backend_name[BUFSIZE];
            scanf("%s%s", printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            cpdbGetAllTranslations(p, locale);
            printTranslations(p);
        }
        else if (strcmp(buf, "get-media-size") == 0)
        {
            char printer_id[BUFSIZE];
            char backend_name[BUFSIZE];
            char media[BUFSIZE];
            int width, length;
            scanf("%s%s%s", media, printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            int ok = cpdbGetMediaSize(p, media, &width, &length);
            if (ok)
                printf("%dx%d\n", width, length);
        }
        else if (strcmp(buf, "get-media-margins") == 0)
        {
            char printer_id[BUFSIZE];
            char backend_name[BUFSIZE];
            char media[BUFSIZE];
            scanf("%s%s%s", media, printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            cpdb_margin_t *margins;
            int num_margins = cpdbGetMediaMargins(p, media, &margins);
            for (int i = 0; i < num_margins; i++)
                printf("%d %d %d %d\n", margins[i].left, margins[i].right, margins[i].top, margins[i].bottom);
        }
        else if (strcmp(buf, "acquire-details") == 0)
        {
            char printer_id[BUFSIZE];
            char backend_name[BUFSIZE];
            scanf("%s%s", printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            if(p == NULL)
              continue;
            g_message("Acquiring printer details asynchronously...\n");
            cpdbAcquireDetails(p, acquire_details_callback, NULL);
		}
        else if (strcmp(buf, "acquire-translations") == 0)
        {
            char printer_id[BUFSIZE];
            char backend_name[BUFSIZE];
            scanf("%s%s", printer_id, backend_name);
            cpdb_printer_obj_t *p = cpdbFindPrinterObj(f, printer_id, backend_name);
            if(p == NULL)
              continue;
            g_message("Acquiring printer translations asynchronously...\n");
            cpdbAcquireTranslations(p, locale, acquire_translations_callback, NULL);
        }
    }
}