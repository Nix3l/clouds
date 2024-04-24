#include "post_processing_shader.h"

#include "game.h"
#include "util/math.h"

static void bind_attributes() {
    shader_s* shader = &game_state->post_processing_shader;
    shader_bind_attribute(shader, 0, "vs_position");
    shader_bind_attribute(shader, 1, "vs_uvs");
}

static void load_uniforms(void* data) {
    shader_s* shader = &game_state->post_processing_shader;
    pproc_shader_uniforms_s* uniforms = &game_state->post_processing_shader_uniforms;

    shader_load_int(uniforms->scene, 0);
    shader_load_int(uniforms->depth, 1);

    shader_load_float(uniforms->near_plane, game_state->camera.near_plane);
    shader_load_float(uniforms->far_plane, game_state->camera.far_plane);

    shader_load_float(uniforms->fog_distance, game_state->fog_distance);
    shader_load_float(uniforms->fog_ramp, game_state->fog_ramp);
    shader_load_float(uniforms->fog_attenuation, game_state->fog_attenuation);
    shader_load_vec3(uniforms->fog_color, game_state->fog_color);

    shader_load_float(uniforms->sun_attenuation, game_state->sun_attenuation);
    shader_load_float(uniforms->sun_intensity, game_state->sun.intensity);
    shader_load_float(uniforms->halo_intensity, game_state->halo_intensity);
    shader_load_vec3(uniforms->sun_color, game_state->sun.color);
    shader_load_vec3(uniforms->sun_dir, game_state->sun.direction);

    shader_load_mat4(uniforms->projection, camera_projection(&game_state->camera));
    shader_load_mat4(uniforms->view, camera_view(&game_state->camera));
}

void init_post_processing_shaders() {
    game_state->post_processing_shader = load_and_create_shader(
            "postprocessing",
            "shader/post_process_vs.glsl",
            "shader/post_process_fs.glsl",
            bind_attributes,
            load_uniforms
        );

    shader_s* shader = &game_state->post_processing_shader;
    pproc_shader_uniforms_s* uniforms = &game_state->post_processing_shader_uniforms;

    uniforms->scene           = shader_get_uniform(shader, "scene_tex");
    uniforms->depth           = shader_get_uniform(shader, "depth_tex");
    uniforms->near_plane      = shader_get_uniform(shader, "near_plane");
    uniforms->far_plane       = shader_get_uniform(shader, "far_plane");

    uniforms->fog_distance    = shader_get_uniform(shader, "fog_distance");
    uniforms->fog_ramp        = shader_get_uniform(shader, "fog_ramp");
    uniforms->fog_attenuation = shader_get_uniform(shader, "fog_attenuation");
    uniforms->fog_color       = shader_get_uniform(shader, "fog_color");

    uniforms->sun_attenuation = shader_get_uniform(shader, "sun_attenuation");
    uniforms->sun_intensity   = shader_get_uniform(shader, "sun_intensity");
    uniforms->halo_intensity  = shader_get_uniform(shader, "halo_intensity");
    uniforms->sun_color       = shader_get_uniform(shader, "sun_color");
    uniforms->sun_dir         = shader_get_uniform(shader, "sun_direction");

    uniforms->projection      = shader_get_uniform(shader, "projection");
    uniforms->view            = shader_get_uniform(shader, "view");
}
