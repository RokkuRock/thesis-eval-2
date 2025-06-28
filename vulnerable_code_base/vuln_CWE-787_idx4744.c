static SDL_Surface* Create_Surface_Solid(int width, int height, SDL_Color fg, Uint32 *color)
{
    const int alignment = Get_Alignement() - 1;
    SDL_Surface *textbuf;
    Sint64 size;
    void *pixels, *ptr;
    Sint64 pitch = width + alignment;
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
    textbuf = SDL_CreateRGBSurfaceWithFormatFrom(pixels, width, height, 0, pitch, SDL_PIXELFORMAT_INDEX8);
    if (textbuf == NULL) {
        SDL_free(ptr);
        return NULL;
    }
    textbuf->flags &= ~SDL_PREALLOC;
    textbuf->flags |= SDL_SIMD_ALIGNED;
    SDL_memset(pixels, 0, height * pitch);
    *color = 1;
    {
        SDL_Palette *palette = textbuf->format->palette;
        palette->colors[0].r = 255 - fg.r;
        palette->colors[0].g = 255 - fg.g;
        palette->colors[0].b = 255 - fg.b;
        palette->colors[1].r = fg.r;
        palette->colors[1].g = fg.g;
        palette->colors[1].b = fg.b;
        palette->colors[1].a = fg.a;
    }
    SDL_SetColorKey(textbuf, SDL_TRUE, 0);
    return textbuf;