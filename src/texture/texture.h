#ifndef TEXTURE_H
#define TEXTURE_H

#include "base.h"
#include "memory/memory.h"

typedef struct {
    // metadata
    char* name;
    char* full_path;

    GLuint id;
} texture_s;

texture_s create_texture(char* filename, arena_s* arena);
// ordered: right, left, top, bottom, front, back
texture_s create_cubemap(char** filenames, arena_s* arena);

void destroy_texture(texture_s* texture);

#endif
