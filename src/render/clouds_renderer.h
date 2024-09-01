#ifndef CLOUDS_RENDERER_H
#define CLOUDS_RENDERER_H

#include "base.h"
#include "clouds/clouds.h"
#include "mesh/mesh.h"
#include "framebuffer/fbo.h"
#include "shader/shader.h"

typedef struct {
    fbo_s back_buffer;

    texture_s blue_noise_tex;
    texture_s weather_map_tex;

    compute_shader_s noise_compute;

    uniform_t u_resolution;
    uniform_t u_worley_cpa;
    uniform_t u_persistence;

    uniform_t u_volume;
} cloud_renderer_s;

void init_cloud_renderer();

void render_cloud_volume(cloud_volume_s* volume, fbo_s* buffer);
void render_cloud_noise(cloud_volume_s* volume, arena_s* arena);

#endif
