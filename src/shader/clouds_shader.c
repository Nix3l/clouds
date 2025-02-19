#include "clouds_shader.h"

#include "game.h"
#include "util/math.h"

static void bind_attributes() {
    shader_bind_attribute(&game_state->cloud_shader.program, 0, "vs_position");
    shader_bind_attribute(&game_state->cloud_shader.program, 1, "vs_uvs");
}

static void load_uniforms(void* _data) {
    cloud_shader_s* shader = &game_state->cloud_shader;
    cloud_volume_s* volume = _data;

    shader_load_int(shader->u_scene_tex, 0);
    shader_load_int(shader->u_depth_tex, 1);

    shader_load_int(shader->u_blue_noise_tex, 2);
    shader_load_int(shader->u_wmap_tex, 3);
    shader_load_int(shader->u_noise_tex, 4);

    shader_load_mat4(shader->u_projection, camera_projection(&game_state->camera));
    shader_load_mat4(shader->u_view, camera_view(&game_state->camera));

    shader_load_float(shader->u_near_plane, game_state->camera.near_plane);
    shader_load_float(shader->u_far_plane, game_state->camera.far_plane);

    shader_load_vec3(shader->u_volume_position, volume->position);
    shader_load_vec3(shader->u_volume_size, volume->size);

    shader_load_vec3(shader->u_light_color, game_state->sun.color);
    shader_load_vec3(shader->u_light_dir, game_state->sun.direction);
    shader_load_float(shader->u_light_intensity, game_state->sun.intensity);

    shader_load_vec3(shader->u_camera_pos, game_state->camera.position);
    shader_load_vec3(shader->u_camera_dir, game_state->camera.rotation);

    shader_load_float(shader->u_time, game_state->curr_time);

    shader_load_int(shader->u_noise_resolution, shader->noise_resolution);

    shader_load_float(shader->u_cloud_scale, shader->cloud_scale);
    shader_load_vec3(shader->u_cloud_offset, shader->cloud_offset);

    shader_load_float(shader->u_global_coverage, shader->global_coverage);
    shader_load_float(shader->u_density_multiplier, shader->density_multiplier);

    shader_load_int(shader->u_march_steps, shader->march_steps);
    shader_load_float(shader->u_step_size, shader->step_size);
    shader_load_int(shader->u_light_march_steps, shader->light_march_steps);

    shader_load_float(shader->u_absorption, shader->absorption);
    shader_load_float(shader->u_phase_coefficient, shader->phase_coefficient);

    shader_load_float(shader->u_edge_falloff, shader->edge_falloff);
    shader_load_float(shader->u_height_falloff, shader->height_falloff);
}

void init_cloud_shader() {
    shader_s shader = load_and_create_shader(
        "shader_cloud",
        "shader/cloud_vs.glsl",
        "shader/cloud_fs.glsl",
        bind_attributes,
        load_uniforms,
        &game_state->frame_arena
    );

    game_state->cloud_shader = (cloud_shader_s) {
        .program = shader,

        .u_scene_tex          = shader_get_uniform(&shader, "scene_tex"),
        .u_depth_tex          = shader_get_uniform(&shader, "depth_tex"),
                              
        .u_blue_noise_tex     = shader_get_uniform(&shader, "blue_noise_tex"),
        .u_wmap_tex           = shader_get_uniform(&shader, "weather_map_tex"),
        .u_noise_tex          = shader_get_uniform(&shader, "noise_tex"),
                              
        .u_projection         = shader_get_uniform(&shader, "projection"),
        .u_view               = shader_get_uniform(&shader, "view"),
                              
        .u_near_plane         = shader_get_uniform(&shader, "near_plane"),
        .u_far_plane          = shader_get_uniform(&shader, "far_plane"),
                              
        .u_volume_position    = shader_get_uniform(&shader, "position"),
        .u_volume_size        = shader_get_uniform(&shader, "size"),

        .u_light_color        = shader_get_uniform(&shader, "light_color"),
        .u_light_dir          = shader_get_uniform(&shader, "light_dir"),
        .u_light_intensity    = shader_get_uniform(&shader, "light_intensity"),

        .u_camera_pos         = shader_get_uniform(&shader, "camera_pos"),
        .u_camera_dir         = shader_get_uniform(&shader, "camera_dir"),

        .u_time               = shader_get_uniform(&shader, "time"),

        .u_noise_resolution   = shader_get_uniform(&shader, "noise_resolution"),
                              
        .u_cloud_scale        = shader_get_uniform(&shader, "cloud_scale"),
        .u_cloud_offset       = shader_get_uniform(&shader, "cloud_offset"),
                              
        .u_global_coverage    = shader_get_uniform(&shader, "global_coverage"),
        .u_density_multiplier = shader_get_uniform(&shader, "density_multiplier"),

        .u_march_steps        = shader_get_uniform(&shader, "march_steps"),
        .u_step_size          = shader_get_uniform(&shader, "march_step_size"),
        .u_light_march_steps  = shader_get_uniform(&shader, "light_march_steps"),

        .u_absorption         = shader_get_uniform(&shader, "absorption"),
        .u_phase_coefficient  = shader_get_uniform(&shader, "phase_coefficient"),

        .u_edge_falloff       = shader_get_uniform(&shader, "edge_falloff"),
        .u_height_falloff     = shader_get_uniform(&shader, "height_falloff"),
    };
}
