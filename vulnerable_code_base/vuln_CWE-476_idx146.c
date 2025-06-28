static int decode_frame(AVCodecContext *avctx,
                        void *data, int *got_frame, AVPacket *avpkt)
{
    TiffContext *const s = avctx->priv_data;
    AVFrame *const p = data;
    ThreadFrame frame = { .f = data };
    unsigned off, last_off;
    int le, ret, plane, planes;
    int i, j, entries, stride;
    unsigned soff, ssize;
    uint8_t *dst;
    GetByteContext stripsizes;
    GetByteContext stripdata;
    int retry_for_subifd, retry_for_page;
    int is_dng;
    int has_tile_bits, has_strip_bits;
    bytestream2_init(&s->gb, avpkt->data, avpkt->size);
    if ((ret = ff_tdecode_header(&s->gb, &le, &off))) {
        av_log(avctx, AV_LOG_ERROR, "Invalid TIFF header\n");
        return ret;
    } else if (off >= UINT_MAX - 14 || avpkt->size < off + 14) {
        av_log(avctx, AV_LOG_ERROR, "IFD offset is greater than image size\n");
        return AVERROR_INVALIDDATA;
    }
    s->le          = le;
    s->tiff_type   = TIFF_TYPE_TIFF;
again:
    s->is_thumbnail = 0;
    s->bppcount    = s->bpp = 1;
    s->photometric = TIFF_PHOTOMETRIC_NONE;
    s->compr       = TIFF_RAW;
    s->fill_order  = 0;
    s->white_level = 0;
    s->is_bayer    = 0;
    s->is_tiled    = 0;
    s->is_jpeg     = 0;
    s->cur_page    = 0;
    s->last_tag    = 0;
    for (i = 0; i < 65536; i++)
        s->dng_lut[i] = i;
    free_geotags(s);
    s->stripsizesoff = s->strippos = 0;
    bytestream2_seek(&s->gb, off, SEEK_SET);
    entries = ff_tget_short(&s->gb, le);
    if (bytestream2_get_bytes_left(&s->gb) < entries * 12)
        return AVERROR_INVALIDDATA;
    for (i = 0; i < entries; i++) {
        if ((ret = tiff_decode_tag(s, p)) < 0)
            return ret;
    }
    if (s->get_thumbnail && !s->is_thumbnail) {
        av_log(avctx, AV_LOG_INFO, "No embedded thumbnail present\n");
        return AVERROR_EOF;
    }
    retry_for_subifd = s->sub_ifd && (s->get_subimage || (!s->get_thumbnail && s->is_thumbnail));
    retry_for_page = s->get_page && s->cur_page + 1 < s->get_page;   
    last_off = off;
    if (retry_for_page) {
        off = ff_tget_long(&s->gb, le);
    } else if (retry_for_subifd) {
        off = s->sub_ifd;
    }
    if (retry_for_subifd || retry_for_page) {
        if (!off) {
            av_log(avctx, AV_LOG_ERROR, "Requested entry not found\n");
            return AVERROR_INVALIDDATA;
        }
        if (off <= last_off) {
            avpriv_request_sample(s->avctx, "non increasing IFD offset");
            return AVERROR_INVALIDDATA;
        }
        if (off >= UINT_MAX - 14 || avpkt->size < off + 14) {
            av_log(avctx, AV_LOG_ERROR, "IFD offset is greater than image size\n");
            return AVERROR_INVALIDDATA;
        }
        s->sub_ifd = 0;
        goto again;
    }
    is_dng = (s->tiff_type == TIFF_TYPE_DNG || s->tiff_type == TIFF_TYPE_CINEMADNG);
    for (i = 0; i<s->geotag_count; i++) {
        const char *keyname = get_geokey_name(s->geotags[i].key);
        if (!keyname) {
            av_log(avctx, AV_LOG_WARNING, "Unknown or unsupported GeoTIFF key %d\n", s->geotags[i].key);
            continue;
        }
        if (get_geokey_type(s->geotags[i].key) != s->geotags[i].type) {
            av_log(avctx, AV_LOG_WARNING, "Type of GeoTIFF key %d is wrong\n", s->geotags[i].key);
            continue;
        }
        ret = av_dict_set(&p->metadata, keyname, s->geotags[i].val, 0);
        if (ret<0) {
            av_log(avctx, AV_LOG_ERROR, "Writing metadata with key '%s' failed\n", keyname);
            return ret;
        }
    }
    if (is_dng) {
        int bps;
        if (s->bpp % s->bppcount)
            return AVERROR_INVALIDDATA;
        bps = s->bpp / s->bppcount;
        if (bps < 8 || bps > 32)
            return AVERROR_INVALIDDATA;
        if (s->white_level == 0)
            s->white_level = (1LL << bps) - 1;  
        if (s->white_level <= s->black_level) {
            av_log(avctx, AV_LOG_ERROR, "BlackLevel (%"PRId32") must be less than WhiteLevel (%"PRId32")\n",
                s->black_level, s->white_level);
            return AVERROR_INVALIDDATA;
        }
        if (s->planar)
            return AVERROR_PATCHWELCOME;
    }
    if (!s->is_tiled && !s->strippos && !s->stripoff) {
        av_log(avctx, AV_LOG_ERROR, "Image data is missing\n");
        return AVERROR_INVALIDDATA;
    }
    has_tile_bits  = s->is_tiled || s->tile_byte_counts_offset || s->tile_offsets_offset || s->tile_width || s->tile_length || s->tile_count;
    has_strip_bits = s->strippos || s->strips || s->stripoff || s->rps || s->sot || s->sstype || s->stripsize || s->stripsizesoff;
    if (has_tile_bits && has_strip_bits) {
        av_log(avctx, AV_LOG_WARNING, "Tiled TIFF is not allowed to strip\n");
    }
    if ((ret = init_image(s, &frame)) < 0)
        return ret;
    if (!s->is_tiled) {
        if (s->strips == 1 && !s->stripsize) {
            av_log(avctx, AV_LOG_WARNING, "Image data size missing\n");
            s->stripsize = avpkt->size - s->stripoff;
        }
        if (s->stripsizesoff) {
            if (s->stripsizesoff >= (unsigned)avpkt->size)
                return AVERROR_INVALIDDATA;
            bytestream2_init(&stripsizes, avpkt->data + s->stripsizesoff,
                            avpkt->size - s->stripsizesoff);
        }
        if (s->strippos) {
            if (s->strippos >= (unsigned)avpkt->size)
                return AVERROR_INVALIDDATA;
            bytestream2_init(&stripdata, avpkt->data + s->strippos,
                            avpkt->size - s->strippos);
        }
        if (s->rps <= 0 || s->rps % s->subsampling[1]) {
            av_log(avctx, AV_LOG_ERROR, "rps %d invalid\n", s->rps);
            return AVERROR_INVALIDDATA;
        }
    }
    if (s->photometric == TIFF_PHOTOMETRIC_LINEAR_RAW ||
        s->photometric == TIFF_PHOTOMETRIC_CFA) {
        p->color_trc = AVCOL_TRC_LINEAR;
    } else if (s->photometric == TIFF_PHOTOMETRIC_BLACK_IS_ZERO) {
        p->color_trc = AVCOL_TRC_GAMMA22;
    }
    if (is_dng && s->is_tiled) {
        if (!s->is_jpeg) {
            avpriv_report_missing_feature(avctx, "DNG uncompressed tiled images");
            return AVERROR_PATCHWELCOME;
        } else if (!s->is_bayer) {
            avpriv_report_missing_feature(avctx, "DNG JPG-compressed tiled non-bayer-encoded images");
            return AVERROR_PATCHWELCOME;
        } else {
            if ((ret = dng_decode_tiles(avctx, (AVFrame*)data, avpkt)) > 0)
                *got_frame = 1;
            return ret;
        }
    }
    planes = s->planar ? s->bppcount : 1;
    for (plane = 0; plane < planes; plane++) {
        uint8_t *five_planes = NULL;
        int remaining = avpkt->size;
        int decoded_height;
        stride = p->linesize[plane];
        dst = p->data[plane];
        if (s->photometric == TIFF_PHOTOMETRIC_SEPARATED &&
            s->avctx->pix_fmt == AV_PIX_FMT_RGBA) {
            stride = stride * 5 / 4;
            five_planes =
            dst = av_malloc(stride * s->height);
            if (!dst)
                return AVERROR(ENOMEM);
        }
        for (i = 0; i < s->height; i += s->rps) {
            if (i)
                dst += s->rps * stride;
            if (s->stripsizesoff)
                ssize = ff_tget(&stripsizes, s->sstype, le);
            else
                ssize = s->stripsize;
            if (s->strippos)
                soff = ff_tget(&stripdata, s->sot, le);
            else
                soff = s->stripoff;
            if (soff > avpkt->size || ssize > avpkt->size - soff || ssize > remaining) {
                av_log(avctx, AV_LOG_ERROR, "Invalid strip size/offset\n");
                av_freep(&five_planes);
                return AVERROR_INVALIDDATA;
            }
            remaining -= ssize;
            if ((ret = tiff_unpack_strip(s, p, dst, stride, avpkt->data + soff, ssize, i,
                                         FFMIN(s->rps, s->height - i))) < 0) {
                if (avctx->err_recognition & AV_EF_EXPLODE) {
                    av_freep(&five_planes);
                    return ret;
                }
                break;
            }
        }
        decoded_height = FFMIN(i, s->height);
        if (s->predictor == 2) {
            if (s->photometric == TIFF_PHOTOMETRIC_YCBCR) {
                av_log(s->avctx, AV_LOG_ERROR, "predictor == 2 with YUV is unsupported");
                return AVERROR_PATCHWELCOME;
            }
            dst   = five_planes ? five_planes : p->data[plane];
            soff  = s->bpp >> 3;
            if (s->planar)
                soff  = FFMAX(soff / s->bppcount, 1);
            ssize = s->width * soff;
            if (s->avctx->pix_fmt == AV_PIX_FMT_RGB48LE ||
                s->avctx->pix_fmt == AV_PIX_FMT_RGBA64LE ||
                s->avctx->pix_fmt == AV_PIX_FMT_GRAY16LE ||
                s->avctx->pix_fmt == AV_PIX_FMT_YA16LE ||
                s->avctx->pix_fmt == AV_PIX_FMT_GBRP16LE ||
                s->avctx->pix_fmt == AV_PIX_FMT_GBRAP16LE) {
                for (i = 0; i < decoded_height; i++) {
                    for (j = soff; j < ssize; j += 2)
                        AV_WL16(dst + j, AV_RL16(dst + j) + AV_RL16(dst + j - soff));
                    dst += stride;
                }
            } else if (s->avctx->pix_fmt == AV_PIX_FMT_RGB48BE ||
                       s->avctx->pix_fmt == AV_PIX_FMT_RGBA64BE ||
                       s->avctx->pix_fmt == AV_PIX_FMT_GRAY16BE ||
                       s->avctx->pix_fmt == AV_PIX_FMT_YA16BE ||
                       s->avctx->pix_fmt == AV_PIX_FMT_GBRP16BE ||
                       s->avctx->pix_fmt == AV_PIX_FMT_GBRAP16BE) {
                for (i = 0; i < decoded_height; i++) {
                    for (j = soff; j < ssize; j += 2)
                        AV_WB16(dst + j, AV_RB16(dst + j) + AV_RB16(dst + j - soff));
                    dst += stride;
                }
            } else {
                for (i = 0; i < decoded_height; i++) {
                    for (j = soff; j < ssize; j++)
                        dst[j] += dst[j - soff];
                    dst += stride;
                }
            }
        }
        if (s->photometric == TIFF_PHOTOMETRIC_WHITE_IS_ZERO) {
            int c = (s->avctx->pix_fmt == AV_PIX_FMT_PAL8 ? (1<<s->bpp) - 1 : 255);
            dst = p->data[plane];
            for (i = 0; i < s->height; i++) {
                for (j = 0; j < stride; j++)
                    dst[j] = c - dst[j];
                dst += stride;
            }
        }
        if (s->photometric == TIFF_PHOTOMETRIC_SEPARATED &&
            (s->avctx->pix_fmt == AV_PIX_FMT_RGB0 || s->avctx->pix_fmt == AV_PIX_FMT_RGBA)) {
            int x = s->avctx->pix_fmt == AV_PIX_FMT_RGB0 ? 4 : 5;
            uint8_t *src = five_planes ? five_planes : p->data[plane];
            dst = p->data[plane];
            for (i = 0; i < s->height; i++) {
                for (j = 0; j < s->width; j++) {
                    int k =  255 - src[x * j + 3];
                    int r = (255 - src[x * j    ]) * k;
                    int g = (255 - src[x * j + 1]) * k;
                    int b = (255 - src[x * j + 2]) * k;
                    dst[4 * j    ] = r * 257 >> 16;
                    dst[4 * j + 1] = g * 257 >> 16;
                    dst[4 * j + 2] = b * 257 >> 16;
                    dst[4 * j + 3] = s->avctx->pix_fmt == AV_PIX_FMT_RGBA ? src[x * j + 4] : 255;
                }
                src += stride;
                dst += p->linesize[plane];
            }
            av_freep(&five_planes);
        } else if (s->photometric == TIFF_PHOTOMETRIC_SEPARATED &&
            s->avctx->pix_fmt == AV_PIX_FMT_RGBA64BE) {
            dst = p->data[plane];
            for (i = 0; i < s->height; i++) {
                for (j = 0; j < s->width; j++) {
                    uint64_t k =  65535 - AV_RB16(dst + 8 * j + 6);
                    uint64_t r = (65535 - AV_RB16(dst + 8 * j    )) * k;
                    uint64_t g = (65535 - AV_RB16(dst + 8 * j + 2)) * k;
                    uint64_t b = (65535 - AV_RB16(dst + 8 * j + 4)) * k;
                    AV_WB16(dst + 8 * j    , r * 65537 >> 32);
                    AV_WB16(dst + 8 * j + 2, g * 65537 >> 32);
                    AV_WB16(dst + 8 * j + 4, b * 65537 >> 32);
                    AV_WB16(dst + 8 * j + 6, 65535);
                }
                dst += p->linesize[plane];
            }
        }
    }
    if (s->planar && s->bppcount > 2) {
        FFSWAP(uint8_t*, p->data[0],     p->data[2]);
        FFSWAP(int,      p->linesize[0], p->linesize[2]);
        FFSWAP(uint8_t*, p->data[0],     p->data[1]);
        FFSWAP(int,      p->linesize[0], p->linesize[1]);
    }
    if (s->is_bayer && s->white_level && s->bpp == 16 && !is_dng) {
        uint16_t *dst = (uint16_t *)p->data[0];
        for (i = 0; i < s->height; i++) {
            for (j = 0; j < s->width; j++)
                dst[j] = FFMIN((dst[j] / (float)s->white_level) * 65535, 65535);
            dst += stride / 2;
        }
    }
    *got_frame = 1;
    return avpkt->size;
}