static void WritePixels(struct ngiflib_img * i, struct ngiflib_decode_context * context, const u8 * pixels, u16 n) {
	u16 tocopy;	
	struct ngiflib_gif * p = i->parent;
	while(n > 0) {
		tocopy = (context->Xtogo < n) ? context->Xtogo : n;
		if(!i->gce.transparent_flag) {
#ifndef NGIFLIB_INDEXED_ONLY
			if(p->mode & NGIFLIB_MODE_INDEXED) {
#endif  
				ngiflib_memcpy(context->frbuff_p.p8, pixels, tocopy);
				pixels += tocopy;
				context->frbuff_p.p8 += tocopy;
#ifndef NGIFLIB_INDEXED_ONLY
			} else {
				int j;
				for(j = (int)tocopy; j > 0; j--) {
					*(context->frbuff_p.p32++) =
					   GifIndexToTrueColor(i->palette, *pixels++);
				}
			}
#endif  
		} else {
			int j;
#ifndef NGIFLIB_INDEXED_ONLY
			if(p->mode & NGIFLIB_MODE_INDEXED) {
#endif  
				for(j = (int)tocopy; j > 0; j--) {
					if(*pixels != i->gce.transparent_color) *context->frbuff_p.p8 = *pixels;
					pixels++;
					context->frbuff_p.p8++;
				}
#ifndef NGIFLIB_INDEXED_ONLY
			} else {
				for(j = (int)tocopy; j > 0; j--) {
					if(*pixels != i->gce.transparent_color) {
						*context->frbuff_p.p32 = GifIndexToTrueColor(i->palette, *pixels);
					}
					pixels++;
					context->frbuff_p.p32++;
				}
			}
#endif  
		}
		context->Xtogo -= tocopy;
		if(context->Xtogo == 0) {
			#ifdef NGIFLIB_ENABLE_CALLBACKS
			if(p->line_cb) p->line_cb(p, context->line_p, context->curY);
			#endif  
			context->Xtogo = i->width;
			switch(context->pass) {
			case 0:
				context->curY++;
				break;
			case 1:	 
				context->curY += 8;
				if(context->curY >= p->height) {
					context->pass++;
					context->curY = i->posY + 4;
				}
				break;
			case 2:	 
				context->curY += 8;
				if(context->curY >= p->height) {
					context->pass++;
					context->curY = i->posY + 2;
				}
				break;
			case 3:	 
				context->curY += 4;
				if(context->curY >= p->height) {
					context->pass++;
					context->curY = i->posY + 1;
				}
				break;
			case 4:	 
				context->curY += 2;
				break;
			}
#ifndef NGIFLIB_INDEXED_ONLY
			if(p->mode & NGIFLIB_MODE_INDEXED) {
#endif  
				#ifdef NGIFLIB_ENABLE_CALLBACKS
				context->line_p.p8 = p->frbuff.p8 + (u32)context->curY*p->width;
				context->frbuff_p.p8 = context->line_p.p8 + i->posX;
				#else
				context->frbuff_p.p8 = p->frbuff.p8 + (u32)context->curY*p->width + i->posX;
				#endif  
#ifndef NGIFLIB_INDEXED_ONLY
			} else {
				#ifdef NGIFLIB_ENABLE_CALLBACKS
				context->line_p.p32 = p->frbuff.p32 + (u32)context->curY*p->width;
				context->frbuff_p.p32 = context->line_p.p32 + i->posX;
				#else
				context->frbuff_p.p32 = p->frbuff.p32 + (u32)context->curY*p->width + i->posX;
				#endif  
			}
#endif  
		}
		n -= tocopy;
	}
}