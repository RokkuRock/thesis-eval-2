rrd_info_t *rrd_graph_v(
    int argc,
    char **argv)
{
    image_desc_t im;
    rrd_info_t *grinfo;
    rrd_graph_init(&im);
    rrd_graph_options(argc, argv, &im);
    if (rrd_test_error()) {
        rrd_info_free(im.grinfo);
        im_free(&im);
        return NULL;
    }
    if (optind >= argc) {
        rrd_info_free(im.grinfo);
        im_free(&im);
        rrd_set_error("missing filename");
        return NULL;
    }
    if (strlen(argv[optind]) >= MAXPATH) {
        rrd_set_error("filename (including path) too long");
        rrd_info_free(im.grinfo);
        im_free(&im);
        return NULL;
    }
    strncpy(im.graphfile, argv[optind], MAXPATH - 1);
    im.graphfile[MAXPATH - 1] = '\0';
    if (strcmp(im.graphfile, "-") == 0) {
        im.graphfile[0] = '\0';
    }
    rrd_graph_script(argc, argv, &im, 1);
    if (rrd_test_error()) {
        rrd_info_free(im.grinfo);
        im_free(&im);
        return NULL;
    }
    if (graph_paint(&im) == -1) {
      rrd_info_free(im.grinfo);
      im_free(&im);
      return NULL;
    }
    if (im.imginfo && *im.imginfo) {
        rrd_infoval_t info;
        char     *path;
        char     *filename;
        if (bad_format_imginfo(im.imginfo)) {
            rrd_info_free(im.grinfo);
            im_free(&im);
            rrd_set_error("bad format for imginfo");
            return NULL;
        }
        path = strdup(im.graphfile);
        filename = basename(path);
        info.u_str =
            sprintf_alloc(im.imginfo,
                          filename,
                          (long) (im.zoom *
                                  im.ximg), (long) (im.zoom * im.yimg));
        grinfo_push(&im, sprintf_alloc("image_info"), RD_I_STR, info);
        free(info.u_str);
        free(path);
    }
    if (im.rendered_image) {
        rrd_infoval_t img;
        img.u_blo.size = im.rendered_image_size;
        img.u_blo.ptr = im.rendered_image;
        grinfo_push(&im, sprintf_alloc("image"), RD_I_BLO, img);
    }
    grinfo = im.grinfo;
    im_free(&im);
    return grinfo;
}