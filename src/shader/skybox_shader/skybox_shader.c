#include "skybox_shader.h"

#include "game.h"

static void bind_attributes() {
    shader_s* shader = &game_state->skybox_shader;
    shader_bind_attribute(shader, 0, "vs_position");
}

static void load_uniforms(void* data) {
    shader_s* shader = &game_state->skybox_shader;
    skybox_shader_uniforms_s* uniforms = &game_state->skybox_shader_uniforms;

    shader_load_int(uniforms->cubemap, 0); // texture slot 0
    
    mat4s projection = camera_projection(&game_state->camera);
    mat4s view = MAT4_IDENTITY;

    view = glms_rotate_x(view, RADIANS(game_state->camera.rotation.x));
    view = glms_rotate_y(view, RADIANS(game_state->camera.rotation.y));
    view = glms_rotate_z(view, RADIANS(game_state->camera.rotation.z));

    shader_load_mat4(uniforms->projection_view, glms_mat4_mul(projection, view));
}

void init_skybox_shader() {
    game_state->skybox_shader = load_and_create_shader(
            "skybox",
            "shader/skybox_vs.glsl",
            "shader/skybox_fs.glsl",
            bind_attributes,
            load_uniforms);

    shader_s* shader = &game_state->skybox_shader;
    skybox_shader_uniforms_s* uniforms = &game_state->skybox_shader_uniforms;

    uniforms->projection_view = shader_get_uniform(shader, "projection_view");
    uniforms->cubemap = shader_get_uniform(shader, "cubemap");
}
