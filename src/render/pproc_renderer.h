#ifndef PPROC_RENDERER_H
#define PPROC_RENDERER_H

#include "framebuffer/fbo.h"
#include "mesh/mesh.h"
#include "shader/shader.h"

typedef struct {
    fbo_s back_buffer;
    mesh_s screen_quad;
} pproc_renderer_s;

void init_pproc_renderer();

void render_post_processing(pproc_renderer_s* renderer, shader_s* shader, fbo_s* fbo);

#endif
