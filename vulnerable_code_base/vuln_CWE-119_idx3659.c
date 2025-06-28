ImagingPcdDecode(Imaging im, ImagingCodecState state, UINT8* buf, int bytes)
{
    int x;
    int chunk;
    UINT8* out;
    UINT8* ptr;
    ptr = buf;
    chunk = 3 * state->xsize;
    for (;;) {
	if (bytes < chunk)
	    return ptr - buf;
	out = state->buffer;
	for (x = 0; x < state->xsize; x++) {
	    out[0] = ptr[x];
	    out[1] = ptr[(x+4*state->xsize)/2];
	    out[2] = ptr[(x+5*state->xsize)/2];
	    out += 4;
	}
	state->shuffle((UINT8*) im->image[state->y],
		       state->buffer, state->xsize);
	if (++state->y >= state->ysize)
	    return -1;  
	out = state->buffer;
	for (x = 0; x < state->xsize; x++) {
	    out[0] = ptr[x+state->xsize];
	    out[1] = ptr[(x+4*state->xsize)/2];
	    out[2] = ptr[(x+5*state->xsize)/2];
	    out += 4;
	}
	state->shuffle((UINT8*) im->image[state->y],
		       state->buffer, state->xsize);
	if (++state->y >= state->ysize)
	    return -1;
	ptr += chunk;
	bytes -= chunk;
    }
}