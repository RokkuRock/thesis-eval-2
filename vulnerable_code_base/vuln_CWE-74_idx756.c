static void dump_wlan_at(unsigned wlanidx, unsigned line) {
	console_goto(t, 0, line);
	console_setcolor(t, 0, BGCOL);
	console_setcolor(t, 1, RGB(255,255,0));
	if(wlanidx == selection) {
		console_printchar(t, '>', 0);
	} else {
		console_printchar(t, ' ', 0);
	}
	struct wlaninfo *w = &wlans[wlanidx];
	lock();
	long long now = getutime64();
	long long age_ms = (now - w->last_seen)/1000;
	age_ms=MIN(5000, age_ms)/100;  
	unsigned a = get_a(age_ms);
	console_setcolor(t, 1, RGB(a,a,a));
	console_goto(t, ESSID_PRINT_START, line);
	char macbuf[18];
	if(*w->essid)
		console_printf(t, "%*s", ESSID_PRINT_LEN, w->essid);
	else
		console_printf(t, "<hidden> %*s", ESSID_PRINT_LEN-9, mac2str(w->mac, macbuf));
	console_goto(t, ESSID_PRINT_END +1, line);
	int scale = max - min;
	int width = dim.w - (ESSID_PRINT_LEN+2);
	unsigned x;
	float widthpercent = (float)width/100.f;
	float scalepercent = (float)scale/100.f;
	float scaleup = (float)width / (float)scale;
	double avg = (double)w->total_rssi/(double)w->count;
	float avg_percent = (avg - (float)min) / scalepercent;
	float curr_percent = ((float)w->last_rssi - (float)min) / scalepercent;
	int avg_marker = (avg - (float)min) * scaleup;
	int curr_marker = ((float)w->last_rssi - (float)min) * scaleup;
	unlock();
	for(x = 0; x < width; x++) {
		rgb_t step_color;
		if(wlanidx == selection) step_color = RGB(get_r(x/widthpercent),get_g(x/widthpercent),0);
		else step_color = RGB(get_r(x/widthpercent),get_r(x/widthpercent),get_r(x/widthpercent));
		console_setcolor(t, 0, step_color);
		if(x != curr_marker) console_setcolor(t, 1, RGB(0,0,0));
		else console_setcolor(t, 1, RGB(255,255,255));
		if(x == avg_marker) console_printchar(t, 'I', 0);
		else if (x == curr_marker) console_printchar(t, '|', 0);
		else if(x == 0) console_printchar(t, '[', 0);
		else if(x == width-1) console_printchar(t, ']', 0);
		else console_printchar(t, ' ', 0);
	}
}