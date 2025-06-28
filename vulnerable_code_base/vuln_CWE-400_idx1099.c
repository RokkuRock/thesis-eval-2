int parse_elf_object(int fd, const char *executable, bool fork_disable_dump, char **ret, JsonVariant **ret_package_metadata) {
        _cleanup_close_pair_ int error_pipe[2] = { -1, -1 }, return_pipe[2] = { -1, -1 }, json_pipe[2] = { -1, -1 };
        _cleanup_(json_variant_unrefp) JsonVariant *package_metadata = NULL;
        _cleanup_free_ char *buf = NULL;
        int r;
        assert(fd >= 0);
        r = dlopen_dw();
        if (r < 0)
                return r;
        r = dlopen_elf();
        if (r < 0)
                return r;
        r = RET_NERRNO(pipe2(error_pipe, O_CLOEXEC|O_NONBLOCK));
        if (r < 0)
                return r;
        if (ret) {
                r = RET_NERRNO(pipe2(return_pipe, O_CLOEXEC));
                if (r < 0)
                        return r;
        }
        if (ret_package_metadata) {
                r = RET_NERRNO(pipe2(json_pipe, O_CLOEXEC));
                if (r < 0)
                        return r;
        }
        r = safe_fork_full("(sd-parse-elf)",
                           (int[]){ fd, error_pipe[1], return_pipe[1], json_pipe[1] },
                           4,
                           FORK_RESET_SIGNALS|FORK_CLOSE_ALL_FDS|FORK_NEW_MOUNTNS|FORK_MOUNTNS_SLAVE|FORK_NEW_USERNS|FORK_WAIT|FORK_REOPEN_LOG,
                           NULL);
        if (r < 0) {
                if (r == -EPROTO) {  
                        int e, k;
                        k = read(error_pipe[0], &e, sizeof(e));
                        if (k < 0 && errno != EAGAIN)  
                                return -errno;
                        if (k == sizeof(e))
                                return e;  
                        if (k != 0)
                                return -EIO;
                }
                return r;
        }
        if (r == 0) {
                if (fork_disable_dump) {
                        r = RET_NERRNO(prctl(PR_SET_DUMPABLE, 0));
                        if (r < 0)
                                goto child_fail;
                }
                r = parse_elf(fd, executable, ret ? &buf : NULL, ret_package_metadata ? &package_metadata : NULL);
                if (r < 0)
                        goto child_fail;
                if (buf) {
                        r = loop_write(return_pipe[1], buf, strlen(buf), false);
                        if (r < 0)
                                goto child_fail;
                        return_pipe[1] = safe_close(return_pipe[1]);
                }
                if (package_metadata) {
                        _cleanup_fclose_ FILE *json_out = NULL;
                        json_out = take_fdopen(&json_pipe[1], "w");
                        if (!json_out) {
                                r = -errno;
                                goto child_fail;
                        }
                        json_variant_dump(package_metadata, JSON_FORMAT_FLUSH, json_out, NULL);
                }
                _exit(EXIT_SUCCESS);
        child_fail:
                (void) write(error_pipe[1], &r, sizeof(r));
                _exit(EXIT_FAILURE);
        }
        error_pipe[1] = safe_close(error_pipe[1]);
        return_pipe[1] = safe_close(return_pipe[1]);
        json_pipe[1] = safe_close(json_pipe[1]);
        if (ret) {
                _cleanup_fclose_ FILE *in = NULL;
                in = take_fdopen(&return_pipe[0], "r");
                if (!in)
                        return -errno;
                r = read_full_stream(in, &buf, NULL);
                if (r < 0)
                        return r;
        }
        if (ret_package_metadata) {
                _cleanup_fclose_ FILE *json_in = NULL;
                json_in = take_fdopen(&json_pipe[0], "r");
                if (!json_in)
                        return -errno;
                r = json_parse_file(json_in, NULL, 0, &package_metadata, NULL, NULL);
                if (r < 0 && r != -ENODATA)  
                        return r;
        }
        if (ret)
                *ret = TAKE_PTR(buf);
        if (ret_package_metadata)
                *ret_package_metadata = TAKE_PTR(package_metadata);
        return 0;
}