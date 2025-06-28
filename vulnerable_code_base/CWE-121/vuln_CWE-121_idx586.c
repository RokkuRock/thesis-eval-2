PrintBackend *cpdbCreateBackendFromFile(GDBusConnection *connection,
                                        const char *backend_file_name)
{
    FILE *file = NULL;
    PrintBackend *proxy;
    GError *error = NULL;
    char *path, *backend_name;
    const char *info_dir_name;
    char obj_path[CPDB_BSIZE];
    backend_name = cpdbGetStringCopy(backend_file_name);
    if ((info_dir_name = getenv("CPDB_BACKEND_INFO_DIR")) == NULL)
      info_dir_name = CPDB_BACKEND_INFO_DIR;
    path = cpdbConcatPath(info_dir_name, backend_file_name);
    if ((file = fopen(path, "r")) == NULL)
    {
        logerror("Error creating backend %s : Couldn't open %s for reading\n",
                    backend_name, path);
        free(path);
        return NULL;
    }
    if (fscanf(file, "%s", obj_path) == 0)
    {
        logerror("Error creating backend %s : Couldn't parse %s\n",
                    backend_name, path);
        free(path);
        fclose(file);
        return NULL;
    }
    free(path);
    fclose(file);
    proxy = print_backend_proxy_new_sync(connection,
                                         0,
                                         backend_name,
                                         obj_path,
                                         NULL,
                                         &error);
    if (error)
    {
        logerror("Error creating backend proxy for %s : %s\n",
                    backend_name, error->message);
        return NULL;
    }
    return proxy;
}