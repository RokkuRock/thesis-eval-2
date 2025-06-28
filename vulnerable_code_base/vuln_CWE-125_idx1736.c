static int vgacon_switch(struct vc_data *c)
{
	int x = c->vc_cols * VGA_FONTWIDTH;
	int y = c->vc_rows * c->vc_font.height;
	int rows = screen_info.orig_video_lines * vga_default_font_height/
		c->vc_font.height;
	vga_video_num_columns = c->vc_cols;
	vga_video_num_lines = c->vc_rows;
	if (!vga_is_gfx) {
		scr_memcpyw((u16 *) c->vc_origin, (u16 *) c->vc_screenbuf,
			    c->vc_screenbuf_size > vga_vram_size ?
				vga_vram_size : c->vc_screenbuf_size);
		if ((vgacon_xres != x || vgacon_yres != y) &&
		    (!(vga_video_num_columns % 2) &&
		     vga_video_num_columns <= screen_info.orig_video_cols &&
		     vga_video_num_lines <= rows))
			vgacon_doresize(c, c->vc_cols, c->vc_rows);
	}
	vgacon_scrollback_switch(c->vc_num);
	return 0;		 
}