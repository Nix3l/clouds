#ifndef CLOUDS_SHADER_H
#define CLOUDS_SHADER_H

#include "shader.h"

typedef struct {
    shader_s program;

    // uniforms
    uniform_t u_scene_tex;
    uniform_t u_depth_tex;

    uniform_t u_blue_noise_tex;
    uniform_t u_noise_tex;

    uniform_t u_projection;
    uniform_t u_view;

    uniform_t u_near_plane;
    uniform_t u_far_plane;

    uniform_t u_volume_position;
    uniform_t u_volume_size;

    uniform_t u_camera_pos;
    uniform_t u_camera_dir;


    uniform_t u_noise_resolution;

    uniform_t u_cloud_scale;
    uniform_t u_cloud_offset;
    uniform_t u_density_threshold;
    uniform_t u_density_multiplier;

    uniform_t u_cloud_march_steps;

    uniform_t u_absorption;

    // parameters 
    i32 noise_resolution;

    f32 cloud_scale;
    v3f cloud_offset;
    f32 density_threshold;
    f32 density_multiplier;

    i32 cloud_march_steps;
    f32 absorption;
} cloud_shader_s;

void init_cloud_shader();

#endif
