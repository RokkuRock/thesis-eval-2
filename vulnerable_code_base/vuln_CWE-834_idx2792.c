static int asf_read_marker(AVFormatContext *s, int64_t size)
{
    AVIOContext *pb = s->pb;
    ASFContext *asf = s->priv_data;
    int i, count, name_len, ret;
    char name[1024];
    avio_rl64(pb);             
    avio_rl64(pb);             
    count = avio_rl32(pb);     
    avio_rl16(pb);             
    name_len = avio_rl16(pb);  
    for (i = 0; i < name_len; i++)
        avio_r8(pb);  
    for (i = 0; i < count; i++) {
        int64_t pres_time;
        int name_len;
        avio_rl64(pb);              
        pres_time = avio_rl64(pb);  
        pres_time -= asf->hdr.preroll * 10000;
        avio_rl16(pb);              
        avio_rl32(pb);              
        avio_rl32(pb);              
        name_len = avio_rl32(pb);   
        if ((ret = avio_get_str16le(pb, name_len * 2, name,
                                    sizeof(name))) < name_len)
            avio_skip(pb, name_len - ret);
        avpriv_new_chapter(s, i, (AVRational) { 1, 10000000 }, pres_time,
                           AV_NOPTS_VALUE, name);
    }
    return 0;
}