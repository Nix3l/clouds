#ifndef SKYBOX_RENDERER_H
#define SKYBOX_RENDERER_H

#include "base.h"

#include "texture/texture.h"
#include "mesh/mesh.h"
#include "framebuffer/fbo.h"

typedef struct {
    mesh_s mesh;
} skybox_renderer_s;

void init_skybox_renderer();

void render_skybox(texture_s* cubemap, fbo_s* framebuffer);

#endif
