#ifndef CLOUDS_RENDERER_H
#define CLOUDS_RENDERER_H

#include "base.h"
#include "clouds/clouds.h"
#include "mesh/mesh.h"
#include "framebuffer/fbo.h"
#include "shader/shader.h"

typedef struct {
    fbo_s back_buffer;

    compute_shader_s noise_compute;
    uniform_t u_cells_per_axis;
} cloud_renderer_s;

void init_cloud_renderer();

void render_cloud_volume(cloud_volume_s* volume, fbo_s* buffer);
void render_cloud_noise(cloud_volume_s* volume, arena_s* arena);

#endif
