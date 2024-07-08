#ifndef CLOUDS_SHADER_H
#define CLOUDS_SHADER_H

#include "shader.h"

typedef struct {
    shader_s program;

    uniform_t u_scene_tex;
    uniform_t u_depth_tex;

    uniform_t u_noise_tex;

    uniform_t u_near_plane;
    uniform_t u_far_plane;

    uniform_t u_projection;
    uniform_t u_view;
} cloud_shader_s;

void init_cloud_shader();

#endif
