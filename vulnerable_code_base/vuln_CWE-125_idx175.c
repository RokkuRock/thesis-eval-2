gst_asf_demux_process_ext_content_desc (GstASFDemux * demux, guint8 * data,
    guint64 size)
{
  GstTagList *taglist;
  guint16 blockcount, i;
  gboolean content3D = FALSE;
  struct
  {
    const gchar *interleave_name;
    GstASF3DMode interleaving_type;
  } stereoscopic_layout_map[] = {
    {
    "SideBySideRF", GST_ASF_3D_SIDE_BY_SIDE_HALF_RL}, {
    "SideBySideLF", GST_ASF_3D_SIDE_BY_SIDE_HALF_LR}, {
    "OverUnderRT", GST_ASF_3D_TOP_AND_BOTTOM_HALF_RL}, {
    "OverUnderLT", GST_ASF_3D_TOP_AND_BOTTOM_HALF_LR}, {
    "DualStream", GST_ASF_3D_DUAL_STREAM}
  };
  GST_INFO_OBJECT (demux, "object is an extended content description");
  taglist = gst_tag_list_new_empty ();
  if (size < 2)
    goto not_enough_data;
  blockcount = gst_asf_demux_get_uint16 (&data, &size);
  for (i = 1; i <= blockcount; ++i) {
    const gchar *gst_tag_name;
    guint16 datatype;
    guint16 value_len;
    guint16 name_len;
    GValue tag_value = { 0, };
    gsize in, out;
    gchar *name;
    gchar *name_utf8 = NULL;
    gchar *value;
    if (!gst_asf_demux_get_string (&name, &name_len, &data, &size))
      goto not_enough_data;
    if (size < 2) {
      g_free (name);
      goto not_enough_data;
    }
    datatype = gst_asf_demux_get_uint16 (&data, &size);
    if (!gst_asf_demux_get_string (&value, &value_len, &data, &size)) {
      g_free (name);
      goto not_enough_data;
    }
    name_utf8 =
        g_convert (name, name_len, "UTF-8", "UTF-16LE", &in, &out, NULL);
    if (name_utf8 != NULL) {
      GST_DEBUG ("Found tag/metadata %s", name_utf8);
      gst_tag_name = gst_asf_demux_get_gst_tag_from_tag_name (name_utf8);
      GST_DEBUG ("gst_tag_name %s", GST_STR_NULL (gst_tag_name));
      switch (datatype) {
        case ASF_DEMUX_DATA_TYPE_UTF16LE_STRING:{
          gchar *value_utf8;
          value_utf8 = g_convert (value, value_len, "UTF-8", "UTF-16LE",
              &in, &out, NULL);
          if (value_utf8 != NULL && *value_utf8 != '\0') {
            GST_DEBUG ("string value %s", value_utf8);
            value_utf8[out] = '\0';
            if (gst_tag_name != NULL) {
              if (strcmp (gst_tag_name, GST_TAG_DATE_TIME) == 0) {
                guint year = atoi (value_utf8);
                if (year > 0) {
                  g_value_init (&tag_value, GST_TYPE_DATE_TIME);
                  g_value_take_boxed (&tag_value, gst_date_time_new_y (year));
                }
              } else if (strcmp (gst_tag_name, GST_TAG_GENRE) == 0) {
                guint id3v1_genre_id;
                const gchar *genre_str;
                if (sscanf (value_utf8, "(%u)", &id3v1_genre_id) == 1 &&
                    ((genre_str = gst_tag_id3_genre_get (id3v1_genre_id)))) {
                  GST_DEBUG ("Genre: %s -> %s", value_utf8, genre_str);
                  g_free (value_utf8);
                  value_utf8 = g_strdup (genre_str);
                }
              } else {
                GType tag_type;
                tag_type = gst_tag_get_type (gst_tag_name);
                g_value_init (&tag_value, tag_type);
                if (!gst_value_deserialize (&tag_value, value_utf8)) {
                  GValue from_val = { 0, };
                  g_value_init (&from_val, G_TYPE_STRING);
                  g_value_set_string (&from_val, value_utf8);
                  if (!g_value_transform (&from_val, &tag_value)) {
                    GST_WARNING_OBJECT (demux,
                        "Could not transform string tag to " "%s tag type %s",
                        gst_tag_name, g_type_name (tag_type));
                    g_value_unset (&tag_value);
                  }
                  g_value_unset (&from_val);
                }
              }
            } else {
              GST_DEBUG ("Setting metadata");
              g_value_init (&tag_value, G_TYPE_STRING);
              g_value_set_string (&tag_value, value_utf8);
              if (content3D) {
                guint i;
                if (strncmp ("StereoscopicLayout", name_utf8,
                        strlen (name_utf8)) == 0) {
                  for (i = 0; i < G_N_ELEMENTS (stereoscopic_layout_map); i++) {
                    if (g_str_equal (stereoscopic_layout_map[i].interleave_name,
                            value_utf8)) {
                      demux->asf_3D_mode =
                          stereoscopic_layout_map[i].interleaving_type;
                      GST_INFO ("find interleave type %u", demux->asf_3D_mode);
                    }
                  }
                }
                GST_INFO_OBJECT (demux, "3d type is %u", demux->asf_3D_mode);
              } else {
                demux->asf_3D_mode = GST_ASF_3D_NONE;
                GST_INFO_OBJECT (demux, "None 3d type");
              }
            }
          } else if (value_utf8 == NULL) {
            GST_WARNING ("Failed to convert string value to UTF8, skipping");
          } else {
            GST_DEBUG ("Skipping empty string value for %s",
                GST_STR_NULL (gst_tag_name));
          }
          g_free (value_utf8);
          break;
        }
        case ASF_DEMUX_DATA_TYPE_BYTE_ARRAY:{
          if (gst_tag_name) {
            if (!g_str_equal (gst_tag_name, GST_TAG_IMAGE)) {
              GST_FIXME ("Unhandled byte array tag %s",
                  GST_STR_NULL (gst_tag_name));
              break;
            } else {
              asf_demux_parse_picture_tag (taglist, (guint8 *) value,
                  value_len);
            }
          }
          break;
        }
        case ASF_DEMUX_DATA_TYPE_DWORD:{
          guint uint_val = GST_READ_UINT32_LE (value);
          g_value_init (&tag_value, G_TYPE_UINT);
          if (!strcmp (name_utf8, "WM/Track"))
            ++uint_val;
          g_value_set_uint (&tag_value, uint_val);
          break;
        }
        case ASF_DEMUX_DATA_TYPE_BOOL:{
          gboolean bool_val = GST_READ_UINT32_LE (value);
          if (strncmp ("Stereoscopic", name_utf8, strlen (name_utf8)) == 0) {
            if (bool_val) {
              GST_INFO_OBJECT (demux, "This is 3D contents");
              content3D = TRUE;
            } else {
              GST_INFO_OBJECT (demux, "This is not 3D contenst");
              content3D = FALSE;
            }
          }
          break;
        }
        default:{
          GST_DEBUG ("Skipping tag %s of type %d", gst_tag_name, datatype);
          break;
        }
      }
      if (G_IS_VALUE (&tag_value)) {
        if (gst_tag_name) {
          GstTagMergeMode merge_mode = GST_TAG_MERGE_APPEND;
          if (strcmp (name_utf8, "WM/TrackNumber") == 0)
            merge_mode = GST_TAG_MERGE_REPLACE;
          gst_tag_list_add_values (taglist, merge_mode, gst_tag_name,
              &tag_value, NULL);
        } else {
          GST_DEBUG ("Setting global metadata %s", name_utf8);
          gst_structure_set_value (demux->global_metadata, name_utf8,
              &tag_value);
        }
        g_value_unset (&tag_value);
      }
    }
    g_free (name);
    g_free (value);
    g_free (name_utf8);
  }
  gst_asf_demux_add_global_tags (demux, taglist);
  return GST_FLOW_OK;
not_enough_data:
  {
    GST_WARNING ("Unexpected end of data parsing ext content desc object");
    gst_tag_list_unref (taglist);
    return GST_FLOW_OK;          
  }
}