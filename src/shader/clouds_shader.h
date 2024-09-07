#ifndef CLOUDS_SHADER_H
#define CLOUDS_SHADER_H

#include "shader.h"

typedef struct {
    shader_s program;

    // uniforms
    uniform_t u_scene_tex;
    uniform_t u_depth_tex;

    uniform_t u_blue_noise_tex;
    uniform_t u_wmap_tex;
    uniform_t u_noise_tex;

    uniform_t u_projection;
    uniform_t u_view;

    uniform_t u_near_plane;
    uniform_t u_far_plane;

    uniform_t u_volume_position;
    uniform_t u_volume_size;

    uniform_t u_light_color;
    uniform_t u_light_dir;
    uniform_t u_light_intensity;

    uniform_t u_camera_pos;
    uniform_t u_camera_dir;

    uniform_t u_time;

    uniform_t u_noise_resolution;

    uniform_t u_cloud_scale;
    uniform_t u_cloud_offset;
    uniform_t u_global_coverage;
    uniform_t u_density_multiplier;

    uniform_t u_march_steps;

    uniform_t u_step_size;
    uniform_t u_light_march_steps;

    uniform_t u_absorption;
    uniform_t u_phase_coefficient;

    uniform_t u_edge_falloff;
    uniform_t u_height_falloff;

    // parameters 
    i32 noise_resolution;

    f32 cloud_scale;
    v3f cloud_offset;
    f32 global_coverage;
    f32 density_multiplier;

    i32 march_steps;
    f32 step_size;
    i32 light_march_steps;

    f32 absorption;
    f32 phase_coefficient;

    f32 edge_falloff;
    f32 height_falloff;
} cloud_shader_s;

void init_cloud_shader();

#endif
