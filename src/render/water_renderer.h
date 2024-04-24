#ifndef WATER_RENDERER_H
#define WATER_RENDERER_H

#include "entity/entity.h"
#include "shader/shader.h"
#include "framebuffer/fbo.h"

// TODO(nix3l): deferred rendering

typedef struct {
    v4f background_color;
    shader_s* shader;

    fbo_s framebuffer;

    bool render_wireframe;
} water_renderer_s;

void init_water_renderer();

void render_water(entity_s* entity);

#endif
