static SDL_Surface *Create_Surface_Blended(int width, int height, SDL_Color fg, Uint32 *color)
{
    const int alignment = Get_Alignement() - 1;
    SDL_Surface *textbuf = NULL;
    Uint32 bgcolor;
    bgcolor = (fg.r << 16) | (fg.g << 8) | fg.b;
    *color = bgcolor | (fg.a << 24);
    if (width != 0) {
        Sint64 size;
        void *pixels, *ptr;
        Sint64 pitch = (width + alignment) * 4;
        pitch += alignment;
        pitch &= ~alignment;
        size = height * pitch + sizeof (void *) + alignment;
        if (size < 0 || size > SDL_MAX_SINT32) {
            return NULL;
        }
        ptr = SDL_malloc((size_t)size);
        if (ptr == NULL) {
            return NULL;
        }
        pixels = (void *)(((uintptr_t)ptr + sizeof(void *) + alignment) & ~alignment);
        ((void **)pixels)[-1] = ptr;
        textbuf = SDL_CreateRGBSurfaceWithFormatFrom(pixels, width, height, 0, pitch, SDL_PIXELFORMAT_ARGB8888);
        if (textbuf == NULL) {
            SDL_free(ptr);
            return NULL;
        }
        textbuf->flags &= ~SDL_PREALLOC;
        textbuf->flags |= SDL_SIMD_ALIGNED;
        SDL_memset4(pixels, bgcolor, (height * pitch) / 4);
        if (fg.a != SDL_ALPHA_OPAQUE) {
            SDL_SetSurfaceBlendMode(textbuf, SDL_BLENDMODE_BLEND);
        }
    }
    return textbuf;