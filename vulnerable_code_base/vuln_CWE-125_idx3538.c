uint32_t _WM_SetupMidiEvent(struct _mdi *mdi, uint8_t * event_data, uint8_t running_event) {
    uint32_t ret_cnt = 0;
    uint8_t command = 0;
    uint8_t channel = 0;
    uint8_t data_1 = 0;
    uint8_t data_2 = 0;
    char *text = NULL;
    if (event_data[0] >= 0x80) {
        command = *event_data & 0xf0;
        channel = *event_data++ & 0x0f;
        ret_cnt++;
    } else {
        command = running_event & 0xf0;
        channel = running_event & 0x0f;
    }
    switch(command) {
        case 0x80:
        _SETUP_NOTEOFF:
            data_1 = *event_data++;
            data_2 = *event_data++;
            _WM_midi_setup_noteoff(mdi, channel, data_1, data_2);
            ret_cnt += 2;
            break;
        case 0x90:
            if (event_data[1] == 0) goto _SETUP_NOTEOFF;  
            data_1 = *event_data++;
            data_2 = *event_data++;
            midi_setup_noteon(mdi, channel, data_1, data_2);
            ret_cnt += 2;
            break;
        case 0xa0:
            data_1 = *event_data++;
            data_2 = *event_data++;
            midi_setup_aftertouch(mdi, channel, data_1, data_2);
            ret_cnt += 2;
            break;
        case 0xb0:
            data_1 = *event_data++;
            data_2 = *event_data++;
            midi_setup_control(mdi, channel, data_1, data_2);
            ret_cnt += 2;
            break;
        case 0xc0:
            data_1 = *event_data++;
            midi_setup_patch(mdi, channel, data_1);
            ret_cnt++;
            break;
        case 0xd0:
            data_1 = *event_data++;
            midi_setup_channel_pressure(mdi, channel, data_1);
            ret_cnt++;
            break;
        case 0xe0:
            data_1 = *event_data++;
            data_2 = *event_data++;
            midi_setup_pitch(mdi, channel, ((data_2 << 7) | (data_1 & 0x7f)));
            ret_cnt += 2;
            break;
        case 0xf0:
            if (channel == 0x0f) {
                uint32_t tmp_length = 0;
                if ((event_data[0] == 0x00) && (event_data[1] == 0x02)) {
                    midi_setup_sequenceno(mdi, ((event_data[2] << 8) + event_data[3]));
                    ret_cnt += 4;
                } else if (event_data[0] == 0x01) {
                    event_data++;
                    ret_cnt++;
                    if (*event_data > 0x7f) {
                        do {
                            tmp_length = (tmp_length << 7) + (*event_data & 0x7f);
                            event_data++;
                            ret_cnt++;
                        } while (*event_data > 0x7f);
                    }
                    tmp_length = (tmp_length << 7) + (*event_data & 0x7f);
                    event_data++;
                    ret_cnt++;
                    text = malloc(tmp_length + 1);
                    memcpy(text, event_data, tmp_length);
                    text[tmp_length] = '\0';
                    midi_setup_text(mdi, text);
                    ret_cnt += tmp_length;
                } else if (event_data[0] == 0x02) {
                    event_data++;
                    ret_cnt++;
                    if (*event_data > 0x7f) {
                        do {
                            tmp_length = (tmp_length << 7) + (*event_data & 0x7f);
                            event_data++;
                            ret_cnt++;
                        } while (*event_data > 0x7f);
                    }
                    tmp_length = (tmp_length << 7) + (*event_data & 0x7f);
                    event_data++;
                    ret_cnt++;
                    if (mdi->extra_info.copyright) {
                        mdi->extra_info.copyright = realloc(mdi->extra_info.copyright,(strlen(mdi->extra_info.copyright) + 1 + tmp_length + 1));
                        memcpy(&mdi->extra_info.copyright[strlen(mdi->extra_info.copyright) + 1], event_data, tmp_length);
                        mdi->extra_info.copyright[strlen(mdi->extra_info.copyright) + 1 + tmp_length] = '\0';
                        mdi->extra_info.copyright[strlen(mdi->extra_info.copyright)] = '\n';
                    } else {
                        mdi->extra_info.copyright = malloc(tmp_length + 1);
                        memcpy(mdi->extra_info.copyright, event_data, tmp_length);
                        mdi->extra_info.copyright[tmp_length] = '\0';
                    }
                    text = malloc(tmp_length + 1);
                    memcpy(text, event_data, tmp_length);
                    text[tmp_length] = '\0';
                    midi_setup_copyright(mdi, text);
                    ret_cnt += tmp_length;
                } else if (event_data[0] == 0x03) {
                    event_data++;
                    ret_cnt++;
                    if (*event_data > 0x7f) {
                        do {
                            tmp_length = (tmp_length << 7) + (*event_data & 0x7f);
                            event_data++;
                            ret_cnt++;
                        } while (*event_data > 0x7f);
                    }
                    tmp_length = (tmp_length << 7) + (*event_data & 0x7f);
                    event_data++;
                    ret_cnt++;
                    text = malloc(tmp_length + 1);
                    memcpy(text, event_data, tmp_length);
                    text[tmp_length] = '\0';
                    midi_setup_trackname(mdi, text);
                    ret_cnt += tmp_length;
                } else if (event_data[0] == 0x04) {
                    event_data++;
                    ret_cnt++;
                    if (*event_data > 0x7f) {
                        do {
                            tmp_length = (tmp_length << 7) + (*event_data & 0x7f);
                            event_data++;
                            ret_cnt++;
                        } while (*event_data > 0x7f);
                    }
                    tmp_length = (tmp_length << 7) + (*event_data & 0x7f);
                    event_data++;
                    ret_cnt++;
                    text = malloc(tmp_length + 1);
                    memcpy(text, event_data, tmp_length);
                    text[tmp_length] = '\0';
                    midi_setup_instrumentname(mdi, text);
                    ret_cnt += tmp_length;
                } else if (event_data[0] == 0x05) {
                    event_data++;
                    ret_cnt++;
                    if (*event_data > 0x7f) {
                        do {
                            tmp_length = (tmp_length << 7) + (*event_data & 0x7f);
                            event_data++;
                            ret_cnt++;
                        } while (*event_data > 0x7f);
                    }
                    tmp_length = (tmp_length << 7) + (*event_data & 0x7f);
                    event_data++;
                    ret_cnt++;
                    text = malloc(tmp_length + 1);
                    memcpy(text, event_data, tmp_length);
                    text[tmp_length] = '\0';
                    midi_setup_lyric(mdi, text);
                    ret_cnt += tmp_length;
                } else if (event_data[0] == 0x06) {
                    event_data++;
                    ret_cnt++;
                    if (*event_data > 0x7f) {
                        do {
                            tmp_length = (tmp_length << 7) + (*event_data & 0x7f);
                            event_data++;
                            ret_cnt++;
                        } while (*event_data > 0x7f);
                    }
                    tmp_length = (tmp_length << 7) + (*event_data & 0x7f);
                    event_data++;
                    ret_cnt++;
                    text = malloc(tmp_length + 1);
                    memcpy(text, event_data, tmp_length);
                    text[tmp_length] = '\0';
                    midi_setup_marker(mdi, text);
                    ret_cnt += tmp_length;
                } else if (event_data[0] == 0x07) {
                    event_data++;
                    ret_cnt++;
                    if (*event_data > 0x7f) {
                        do {
                            tmp_length = (tmp_length << 7) + (*event_data & 0x7f);
                            event_data++;
                            ret_cnt++;
                        } while (*event_data > 0x7f);
                    }
                    tmp_length = (tmp_length << 7) + (*event_data & 0x7f);
                    event_data++;
                    ret_cnt++;
                    text = malloc(tmp_length + 1);
                    memcpy(text, event_data, tmp_length);
                    text[tmp_length] = '\0';
                    midi_setup_cuepoint(mdi, text);
                    ret_cnt += tmp_length;
                } else if ((event_data[0] == 0x20) && (event_data[1] == 0x01)) {
                    midi_setup_channelprefix(mdi, event_data[2]);
                    ret_cnt += 3;
                } else if ((event_data[0] == 0x21) && (event_data[1] == 0x01)) {
                    midi_setup_portprefix(mdi, event_data[2]);
                    ret_cnt += 3;
                } else if ((event_data[0] == 0x2F) && (event_data[1] == 0x00)) {
                    _WM_midi_setup_endoftrack(mdi);
                    ret_cnt += 2;
                } else if ((event_data[0] == 0x51) && (event_data[1] == 0x03)) {
                    _WM_midi_setup_tempo(mdi, ((event_data[2] << 16) + (event_data[3] << 8) + event_data[4]));
                    ret_cnt += 5;
                } else if ((event_data[0] == 0x54) && (event_data[1] == 0x05)) {
                    midi_setup_smpteoffset(mdi, ((event_data[3] << 24) + (event_data[4] << 16) + (event_data[5] << 8) + event_data[6]));
                    mdi->events[mdi->events_size - 1].event_data.channel = event_data[2];
                    ret_cnt += 7;
                } else if ((event_data[0] == 0x58) && (event_data[1] == 0x04)) {
                    midi_setup_timesignature(mdi, ((event_data[2] << 24) + (event_data[3] << 16) + (event_data[4] << 8) + event_data[5]));
                    ret_cnt += 6;
                } else if ((event_data[0] == 0x59) && (event_data[1] == 0x02)) {
                    midi_setup_keysignature(mdi, ((event_data[2] << 8) + event_data[3]));
                    ret_cnt += 4;
                } else {
                    event_data++;
                    ret_cnt++;
                    if (*event_data > 0x7f) {
                        do {
                            tmp_length = (tmp_length << 7) + (*event_data & 0x7f);
                            event_data++;
                            ret_cnt++;
                        } while (*event_data > 0x7f);
                    }
                    tmp_length = (tmp_length << 7) + (*event_data & 0x7f);
                    ret_cnt++;
                    ret_cnt += tmp_length;
                }
            } else if ((channel == 0) || (channel == 7)) {
                uint32_t sysex_len = 0;
                uint8_t *sysex_store = NULL;
                if (*event_data > 0x7f) {
                    do {
                        sysex_len = (sysex_len << 7) + (*event_data & 0x7F);
                        event_data++;
                        ret_cnt++;
                    } while (*event_data > 0x7f);
                }
                sysex_len = (sysex_len << 7) + (*event_data & 0x7F);
                event_data++;
                if (!sysex_len) break;
                ret_cnt++;
                sysex_store = malloc(sizeof(uint8_t) * sysex_len);
                memcpy(sysex_store, event_data, sysex_len);
                if (sysex_store[sysex_len - 1] == 0xF7) {
                    uint8_t rolandsysexid[] = { 0x41, 0x10, 0x42, 0x12 };
                    if (memcmp(rolandsysexid, sysex_store, 4) == 0) {
                        uint8_t sysex_cs = 0;
                        uint32_t sysex_ofs = 4;
                        do {
                            sysex_cs += sysex_store[sysex_ofs];
                            if (sysex_cs > 0x7F) {
                                sysex_cs -= 0x80;
                            }
                            sysex_ofs++;
                        } while (sysex_store[sysex_ofs + 1] != 0xf7);
                        sysex_cs = 128 - sysex_cs;
                        if (sysex_cs == sysex_store[sysex_ofs]) {
                            if (sysex_store[4] == 0x40) {
                                if (((sysex_store[5] & 0xf0) == 0x10) && (sysex_store[6] == 0x15)) {
                                    uint8_t sysex_ch = 0x0f & sysex_store[5];
                                    if (sysex_ch == 0x00) {
                                        sysex_ch = 0x09;
                                    } else if (sysex_ch <= 0x09) {
                                        sysex_ch -= 1;
                                    }
                                    midi_setup_sysex_roland_drum_track(mdi, sysex_ch, sysex_store[7]);
                                } else if ((sysex_store[5] == 0x00) && (sysex_store[6] == 0x7F) && (sysex_store[7] == 0x00)) {
                                    midi_setup_sysex_roland_reset(mdi);
                                }
                            }
                        }
                    } else {
                        uint8_t gm_reset[] = {0x7e, 0x7f, 0x09, 0x01, 0xf7};
                        uint8_t yamaha_reset[] = {0x43, 0x10, 0x4c, 0x00, 0x00, 0x7e, 0x00, 0xf7};
                        if (memcmp(gm_reset, sysex_store, 5) == 0) {
                            midi_setup_sysex_gm_reset(mdi);
                        } else if (memcmp(yamaha_reset,sysex_store,8) == 0) {
                            midi_setup_sysex_yamaha_reset(mdi);
                        }
                    }
                }
                free(sysex_store);
                sysex_store = NULL;
                ret_cnt += sysex_len;
            } else {
                _WM_GLOBAL_ERROR(__FUNCTION__, __LINE__, WM_ERR_CORUPT, "(unrecognized meta type event)", 0);
                return 0;
            }
            break;
        default:  
            ret_cnt = 0;
            break;
    }
    if (ret_cnt == 0)
        _WM_GLOBAL_ERROR(__FUNCTION__, __LINE__, WM_ERR_CORUPT, "(missing event)", 0);
    return ret_cnt;
}